#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>

#include "fat.h"
#include "sys_log.h"

#define VALID_BOOT_SIGNATURE 0xaa55
#define VALID_JUMP_BOOT0 0xeb
#define VALID_JUMP_BOOT2 0x90

#define FAT12_MAX_CLUSTERS 4085
#define FAT16_MAX_CLUSTERS 65525

#define FAT16_BOOT_SECTOR_SIZE 62
#define FAT32_BOOT_SECTOR_SIZE 90

#define ATTR_READ_ONLY    (0b1 << 0)
#define ATTR_HIDDEN       (0b1 << 1)
#define ATTR_SYSTEM       (0b1 << 2)
#define ATTR_VOLUME_ID    (0b1 << 3)
#define ATTR_DIRECTORY    (0b1 << 4)
#define ATTR_ARCHIVE      (0b1 << 5)
#define ATTR_LONG_NAME (ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID)

static file_allocation_table_t fat;
static fat_inode_t system_inode_table[MAX_OPEN_FILES];
static uint8_t invalid_bytes[] = {
  0x00, 0x22, 0x2a, 0x2b, 0x2c, 0x2f, 0x3a, 0x3b,
  0x3c, 0x3d, 0x3e, 0x3f, 0x5b, 0x5c, 0x5d, 0x7c
};

status_t fat_print_entry(fat_directory_entry_t entry){
  char name[FAT_DIR_ENTRY_NAME_LEN + 1]; // +1 for '\0'
  for(uint32_t i = 0; i < FAT_DIR_ENTRY_NAME_LEN; ++i){
    name[i] = entry.name[i];
  }
  name[FAT_DIR_ENTRY_NAME_LEN] = '\0';

  SYS_LOGD(
    "directory entry:\n"
    "    name: %s\n"
    "    attribute: %#x\n"
    "    creation_time_tenth: %#x\n"
    "    creation_time (h,m,s): (%d,%d,%d)\n"
    "    creation_date (y,m,d): (%d,%d,%d)\n"
    "    last_access_date (y,m,d): (%d,%d,%d)\n"
    "    last_write_time (h,m,s): (%d,%d,%d)\n"
    "    last_write_date (y,m,d): (%d,%d,%d)\n"
    "    first_cluster: %#x\n"
    "    file_size: %d\n"
    "    reserved: %#x",
    name,
    entry.attribute,
    entry.creation_time_tenth,
    entry.creation_time.hours,
    entry.creation_time.minutes,
    entry.creation_time.two_seconds,
    entry.creation_date.year,
    entry.creation_date.month,
    entry.creation_date.day,
    entry.last_access_date.year,
    entry.last_access_date.month,
    entry.last_access_date.day,
    entry.last_write_time.hours,
    entry.last_write_time.minutes,
    entry.last_write_time.two_seconds,
    entry.last_write_date.year,
    entry.last_write_date.month,
    entry.last_write_date.day,
    entry.first_cluster_low,
    entry.file_size,
    entry.reserved0
  );

  return STATUS_OK;
}

status_t fat_print_bpb(){
  bios_parameter_block_t* bpb = &fat.bs16.fields.bpb;
  char name[9]; // +1 for '\0'
  for(uint32_t i = 0; i < 8; ++i){
    name[i] = bpb->oem_name[i];
    SYS_LOG("%c %c", name[i], bpb->oem_name[i]);

  }
  name[8] = '\0';

  SYS_LOGD(
    "bios parameter block:\n"
    "    jump_boot: %#x\n"
    "    name: %s\n" 
    "    bytes_per_sector: %#x\n"
    "    sectors_per_cluster: %#x\n"
    "    reserved_sector_count: %#x\n"
    "    number_of_fats: %#x\n"
    "    root_entry_count: %#x\n"
    "    total_sector_count_16bit: %#x\n"
    "    media: %#x\n"
    "    fat_sector_count_16bit: %#x\n"
    "    sectors_per_track: %#x\n"
    "    number_of_heads: %#x\n"
    "    hidden_sector_count: %#x\n"
    "    total_sector_count_32bit: %#x\n",
    bpb->jump_boot[0] + (bpb->jump_boot[1] << 8) + (bpb->jump_boot[2] << 16),
    name,
    bpb->bytes_per_sector,
    bpb->sectors_per_cluster,
    bpb->reserved_sector_count,
    bpb->number_of_fats,
    bpb->root_entry_count,
    bpb->total_sector_count_16bit,
    bpb->media,
    bpb->fat_sector_count_16bit,
    bpb->sectors_per_track,
    bpb->number_of_heads,
    bpb->hidden_sector_count,
    bpb->total_sector_count_32bit
  );
  return STATUS_OK;
}

status_t fat_init(){
  STATUS_OK_OR_RETURN(emmc_init());

  emmc_block_t block;
  STATUS_OK_OR_RETURN(emmc_read_block(0, 1, &block));

  memcpy(fat.mbr.raw, block.buf, EMMC_BLOCK_SIZE);

  if(fat.mbr.fields.boot_signature != VALID_BOOT_SIGNATURE){
    SYS_LOGE("invalid boot signature: %#x", fat.mbr.fields.boot_signature);
    return STATUS_ERR;
  }

  fat.partition_base_sector = fat.mbr.fields.p1.lba_first_sector_address;
  STATUS_OK_OR_RETURN(emmc_read_block(fat.partition_base_sector, 1, &block));
  SYS_LOGD("base_sec: %d", fat.partition_base_sector);

  memcpy(&fat.bs16.raw[0], block.buf, FAT16_BOOT_SECTOR_SIZE);
  memcpy(&fat.bs32.raw[0], block.buf, FAT32_BOOT_SECTOR_SIZE);

  bios_parameter_block_t* bpb = &fat.bs32.fields.bpb;
  if((bpb->jump_boot[0] != VALID_JUMP_BOOT0) || (bpb->jump_boot[2] != VALID_JUMP_BOOT2)){
    SYS_LOGE("invalid jump_boot: %#x %#x %#x", bpb->jump_boot[0], bpb->jump_boot[1], bpb->jump_boot[2]); 
    return STATUS_ERR;
  }

  uint32_t fat_sector_count = 0;
  if(bpb->fat_sector_count_16bit != 0){
    fat_sector_count = bpb->fat_sector_count_16bit;
  }else if(fat.bs32.fields.fat_sector_count_32bit != 0){
    fat_sector_count = fat.bs32.fields.fat_sector_count_32bit; 
  }else{
    SYS_LOGE("fat sector count cannot be 0");
    return STATUS_ERR;
  }
 
  uint32_t total_sector_count = 0;
  if(bpb->total_sector_count_16bit != 0){
    total_sector_count = bpb->total_sector_count_16bit;
  }else if(bpb->total_sector_count_32bit != 0){
    total_sector_count = bpb->total_sector_count_32bit;
  }else{
    SYS_LOGE("total sector count cannot be 0");
    return STATUS_ERR;
  }

  uint32_t root_dir_sector_count = ((bpb->root_entry_count * 32) + (bpb->bytes_per_sector - 1)) / bpb->bytes_per_sector;
  uint32_t data_sectors = total_sector_count - (bpb->reserved_sector_count + (bpb->number_of_fats * fat_sector_count) + root_dir_sector_count);
  uint32_t count_of_clusters = data_sectors / bpb->sectors_per_cluster;
  SYS_LOGD("count of clusters: %#x", count_of_clusters);

  fat.blocks_per_cluster = bpb->sectors_per_cluster * (bpb->bytes_per_sector / EMMC_BLOCK_SIZE);
  if(count_of_clusters < FAT12_MAX_CLUSTERS){
    SYS_LOGE("FAT12 not supported"); // as of right now
    return STATUS_ERR;
  }else if(count_of_clusters < FAT16_MAX_CLUSTERS){
    SYS_LOGD("FAT type is FAT16");
    fat.type = FAT16;
    fat.fat_base_sector = bpb->reserved_sector_count;
    fat.root_dir_base_sector = fat.fat_base_sector + (bpb->number_of_fats * bpb->fat_sector_count_16bit);
    fat.data_base_sector =  fat.root_dir_base_sector + root_dir_sector_count;
  }else{
    SYS_LOGD("FAT32 not supported"); // as of right now
    return STATUS_ERR;
    /*
    fat.type = FAT32;
    fat.fat_base_sector = bpb->reserved_sector_count;
    fat.root_dir_base_sector = fat.bs32.fields.root_cluster * bpb->sectors_per_cluster;
    */
  }
  SYS_LOGV("fat init done");

  fat_print_bpb();

  for(uint32_t i = 0; i < MAX_OPEN_FILES; ++i){
    system_inode_table[i].dir_entry.name[0] = 0;
  }
  return STATUS_OK;
}

status_t fat_validate_file_name(const char* file_name){ 
  if((file_name[0] != 0x5) && ((file_name[0] == 0xE5) || (file_name[0] == '.') || (file_name[0] <= ' '))){
    SYS_LOG("invalid file name: %s", file_name);
    return STATUS_ERR;
  }

  for(uint32_t i = 0; i < sizeof(invalid_bytes); ++i){
    if(file_name[0] == invalid_bytes[i]){
      SYS_LOG("invalid file name: %s", file_name);
      return STATUS_ERR;
    }
  }

  int file_name_len = strlen(file_name);
  for(uint32_t i = 1; i < file_name_len; ++i){
    if(file_name[i] < ' '){
      SYS_LOG("invalid file name: %s", file_name);
      return STATUS_ERR;
    }
    for(uint32_t j = 0; j < sizeof(invalid_bytes); ++j){
      if(file_name[i] == invalid_bytes[j]){
        SYS_LOG("invalid file name: %s", file_name);
        return STATUS_ERR;
      }
    }
  }

  return STATUS_OK;
}

status_t fat_convert_file_name(const char* file_name, fat_directory_entry_t* dir_entry){ 
  if(strlen(file_name) > MAX_FILE_NAME_LEN){
    SYS_LOG("invalid file name: %s", file_name);
    return STATUS_ERR;
  }

  STATUS_OK_OR_RETURN(fat_validate_file_name(file_name)); 

  const char dot[2] = ".";
  char file_name_copy[MAX_FILE_NAME_LEN];
  strcpy(file_name_copy, file_name);
  char* main_name = strtok(file_name_copy, dot);
  char* extension = strtok(NULL, dot);
  if(strtok(NULL, dot) != NULL){
    SYS_LOGE("invalid file path: %s", file_name);
    return STATUS_ERR;
  }

  if(strlen(main_name) > 8){
    SYS_LOGE("invalid main name: %s", main_name);
    return STATUS_ERR;
  }else if(strlen(extension) > 3){
    SYS_LOGE("invalid extension: %s", extension);
    return STATUS_ERR;
  }

  for(uint32_t i = 0; i < FAT_DIR_ENTRY_NAME_LEN; ++i){
    if(i < strlen(main_name)){
      dir_entry->name[i] = toupper(main_name[i]);
    }else if(i < 8){
      dir_entry->name[i] = ' ';
    }else if(i < (8 + strlen(extension))){
      dir_entry->name[i] = toupper(extension[i - 8]);
    }else{
      dir_entry->name[i] = ' ';
    }
  }

  return STATUS_OK;
}

status_t fat_get_dir_entry(const char* file_name, fat_directory_entry_t* dir_entry){
  char dir_entry_name[FAT_DIR_ENTRY_NAME_LEN + 1];
  STATUS_OK_OR_RETURN(fat_convert_file_name(file_name, dir_entry));
  memcpy(dir_entry_name, dir_entry, FAT_DIR_ENTRY_NAME_LEN);
  dir_entry_name[FAT_DIR_ENTRY_NAME_LEN] = '\0';

  bios_parameter_block_t bpb = fat.bs16.fields.bpb;
  uint32_t root_dir_sector_count = ((bpb.root_entry_count * sizeof(fat_directory_entry_t)) + (bpb.bytes_per_sector - 1)) / bpb.bytes_per_sector;
  
  emmc_block_t block;
  for(uint32_t i = 0; i < root_dir_sector_count; ++i){
    emmc_read_block(fat.partition_base_sector + fat.root_dir_base_sector + i, 1, &block);
    uint8_t* block_buf = (uint8_t*)block.buf;
    for(uint32_t j = 0; j < (EMMC_BLOCK_SIZE / sizeof(fat_directory_entry_t)); ++j){
      memcpy(dir_entry, block_buf + (j * (sizeof(fat_directory_entry_t))), sizeof(fat_directory_entry_t));
      if(dir_entry->name[0] != 0x00 && dir_entry->name[0] != 0xe5){
        if(strncmp(dir_entry->name, dir_entry_name, FAT_DIR_ENTRY_NAME_LEN) == 0){
          return STATUS_OK;
        }
      }
    }
  }

  memset(dir_entry, 0x0, sizeof(fat_directory_entry_t));
  return STATUS_ERR;
}

status_t fat_set_dir_entry(const char* file_name, fat_directory_entry_t* dir_entry){
  fat_directory_entry_t tmp_dir_entry;
  char dir_entry_name[FAT_DIR_ENTRY_NAME_LEN + 1];
  STATUS_OK_OR_RETURN(fat_convert_file_name(file_name, &tmp_dir_entry));
  memcpy(dir_entry_name, tmp_dir_entry.name, FAT_DIR_ENTRY_NAME_LEN);
  dir_entry_name[FAT_DIR_ENTRY_NAME_LEN] = '\0';

  bios_parameter_block_t bpb = fat.bs16.fields.bpb;
  uint32_t root_dir_sector_count = ((bpb.root_entry_count * sizeof(fat_directory_entry_t)) + (bpb.bytes_per_sector - 1)) / bpb.bytes_per_sector;
  
  emmc_block_t block;
  fat_directory_entry_t search_dir_entry;
  for(uint32_t i = 0; i < root_dir_sector_count; ++i){
    emmc_read_block(fat.partition_base_sector + fat.root_dir_base_sector + i, 1, &block);
    uint8_t* block_buf = (uint8_t*)block.buf;
    for(uint32_t j = 0; j < (EMMC_BLOCK_SIZE / sizeof(fat_directory_entry_t)); ++j){
      memcpy(&search_dir_entry, block_buf + (j * (sizeof(fat_directory_entry_t))), sizeof(fat_directory_entry_t));
      if(search_dir_entry.name[0] != 0x00 && search_dir_entry.name[0] != 0xe5){
        if(strncmp(search_dir_entry.name, dir_entry_name, FAT_DIR_ENTRY_NAME_LEN) == 0){ 
          memcpy(block_buf + (j * (sizeof(fat_directory_entry_t))), dir_entry, sizeof(fat_directory_entry_t));
          emmc_write_block(fat.partition_base_sector + fat.root_dir_base_sector + i, 1, &block);
          return STATUS_OK;
        }
      }
    }
  }

  return STATUS_ERR;
}

status_t fat_find_free_cluster(uint32_t* cluster){
  SYS_LOGV("partition_base_sector: %#x, base_sector: %#x", fat.partition_base_sector, fat.fat_base_sector);
  bios_parameter_block_t bpb = fat.bs16.fields.bpb;
  emmc_block_t block;
  SYS_LOGV("sector_count: %d", bpb.fat_sector_count_16bit);
  for(uint32_t i = 0; i < bpb.fat_sector_count_16bit; ++i){
    emmc_read_block(fat.partition_base_sector + fat.fat_base_sector + i, 1, &block);
    uint16_t* fat_block = (uint16_t*)block.buf;
    for(uint32_t j = 0; j < (EMMC_BLOCK_SIZE / sizeof(uint16_t)); ++j){
      if(fat_block[j] == 0x0){
        *cluster = j + i * (EMMC_BLOCK_SIZE / sizeof(uint16_t));
        SYS_LOGV("free cluster: %d", *cluster);
        fat_block[j] = 0xffff;
        STATUS_OK_OR_RETURN(emmc_write_block(fat.partition_base_sector + fat.fat_base_sector + i, 1, &block));
        return STATUS_OK;
      }
    }
  }

  return STATUS_ERR;
}

status_t fat_create_file(const char* file_name, fat_directory_entry_t* dir_entry){
  SYS_LOGV("creating file %s", file_name);
  STATUS_OK_OR_RETURN(fat_convert_file_name(file_name, dir_entry));

  dir_entry->file_size = 0;
  dir_entry->attribute = ATTR_ARCHIVE;
  dir_entry->reserved0 = 0x18; // indicates lowercase filename

  bios_parameter_block_t* bpb = &fat.bs16.fields.bpb;
  uint32_t root_dir_sector_count = ((bpb->root_entry_count * sizeof(fat_directory_entry_t)) + (bpb->bytes_per_sector - 1)) / bpb->bytes_per_sector; 


  // find free directory entry
  fat_directory_entry_t search_dir_entry = {0};
  emmc_block_t block;
  for(uint32_t i = 0; i < root_dir_sector_count; ++i){
    emmc_read_block(fat.partition_base_sector + fat.root_dir_base_sector + i, 1, &block);
    uint8_t* block_buf = (uint8_t*)block.buf;
    for(uint32_t j = 0; j < (EMMC_BLOCK_SIZE / sizeof(fat_directory_entry_t)); ++j){
      memcpy(&search_dir_entry, block_buf + (j * (sizeof(fat_directory_entry_t))), sizeof(fat_directory_entry_t));
      if(search_dir_entry.name[0] == 0x00 || search_dir_entry.name[0] == 0xe5){
        uint32_t cluster = 0;
        STATUS_OK_OR_RETURN(fat_find_free_cluster(&cluster));
        dir_entry->first_cluster_low = cluster;
        SYS_LOGV("cluster: %d, %d", cluster, dir_entry->first_cluster_low);
        memcpy(block_buf + (j * (sizeof(fat_directory_entry_t))), dir_entry, sizeof(fat_directory_entry_t));
        STATUS_OK_OR_RETURN(emmc_write_block(fat.partition_base_sector + fat.root_dir_base_sector + i, 1, &block));
        return STATUS_OK;
      }
    }
  } 
  return STATUS_ERR;
}

status_t fat_get_absolute_cluster(fat_directory_entry_t* dir_entry, uint32_t file_cluster_offset, uint32_t* absolute_cluster){
  // the first cluster of the file is also the offset into the FAT
  uint32_t file_first_cluster = dir_entry->first_cluster_low + (dir_entry->first_cluster_high << 16);
  SYS_LOGV("first_cluster: %d", file_first_cluster);
  emmc_block_t block;
  uint16_t* fat_block;
  uint32_t fat_sector_offset = 0;
  uint32_t fat_entry_offset = 0;
  *absolute_cluster = file_first_cluster;
  for(uint32_t i = 0; i < file_cluster_offset; ++i){
    if(*absolute_cluster == 0xffff){
      SYS_LOGV("file ended too early: i = %d", i);
      return STATUS_ERR;
    }
    fat_sector_offset = *absolute_cluster / (EMMC_BLOCK_SIZE / sizeof(uint16_t));
    fat_entry_offset = *absolute_cluster  % (EMMC_BLOCK_SIZE / sizeof(uint16_t));
    STATUS_OK_OR_RETURN(emmc_read_block(fat.partition_base_sector + fat.fat_base_sector + fat_sector_offset, 1, &block));
    fat_block = (uint16_t*)block.buf;
    *absolute_cluster = fat_block[fat_entry_offset];
    SYS_LOGV("fat_entry_offset: %#x, absolute_cluster: %#x", fat_entry_offset, *absolute_cluster);
  }
 
  return STATUS_OK;
}

status_t fat_append_cluster(fat_directory_entry_t* dir_entry){
  bios_parameter_block_t* bpb = &fat.bs16.fields.bpb;
  uint32_t num_blocks = (dir_entry->file_size / EMMC_BLOCK_SIZE);
  uint32_t cluster_offset = num_blocks / bpb->sectors_per_cluster;
  uint32_t free_cluster = 0xffff;
  uint32_t absolute_cluster = 0xffff;
  STATUS_OK_OR_RETURN(fat_find_free_cluster(&free_cluster));
  STATUS_OK_OR_RETURN(fat_get_absolute_cluster(dir_entry, cluster_offset - 1, &absolute_cluster));
  SYS_LOGV("absolute cluster: %d", absolute_cluster);

  uint32_t fat_sector_offset = absolute_cluster / (EMMC_BLOCK_SIZE / sizeof(uint16_t));
  uint32_t fat_entry_offset = absolute_cluster  % (EMMC_BLOCK_SIZE / sizeof(uint16_t));
  emmc_block_t block;
  STATUS_OK_OR_RETURN(emmc_read_block(fat.partition_base_sector + fat.fat_base_sector + fat_sector_offset, 1, &block));
  uint16_t* fat_block = (uint16_t*)block.buf;
  fat_block[fat_entry_offset] = free_cluster;
  STATUS_OK_OR_RETURN(emmc_write_block(fat.partition_base_sector + fat.fat_base_sector + fat_sector_offset, 1, &block));
  return STATUS_OK;
}

status_t fat_read_block(fat_directory_entry_t* dir_entry, uint32_t file_block_number, emmc_block_t* block){
  bios_parameter_block_t* bpb = &fat.bs16.fields.bpb;
  uint32_t cluster_offset = file_block_number / bpb->sectors_per_cluster;
  uint32_t sector_offset = file_block_number % bpb->sectors_per_cluster;

  uint32_t absolute_cluster;
  STATUS_OK_OR_RETURN(fat_get_absolute_cluster(dir_entry, cluster_offset, &absolute_cluster));

  uint32_t absolute_sector = fat.partition_base_sector + fat.data_base_sector + (absolute_cluster - 2) * bpb->sectors_per_cluster + sector_offset;
  SYS_LOGV("absolute sector: %#x", absolute_sector);

  STATUS_OK_OR_RETURN(emmc_read_block(absolute_sector, 1, block));
  return STATUS_OK;
}

status_t fat_write_block(fat_directory_entry_t* dir_entry, uint32_t file_block_number, emmc_block_t* block){
  bios_parameter_block_t* bpb = &fat.bs16.fields.bpb;
  uint32_t cluster_offset = file_block_number / bpb->sectors_per_cluster;
  uint32_t sector_offset = file_block_number % bpb->sectors_per_cluster;

  uint32_t absolute_cluster;
  STATUS_OK_OR_RETURN(fat_get_absolute_cluster(dir_entry, cluster_offset, &absolute_cluster));

  uint32_t absolute_sector = fat.partition_base_sector + fat.data_base_sector + (absolute_cluster - 2) * bpb->sectors_per_cluster + sector_offset;
  SYS_LOGV("absolute sector: %#x", absolute_sector);

  STATUS_OK_OR_RETURN(emmc_write_block(absolute_sector, 1, block));
  return STATUS_OK;
}

status_t fat_append_block(fat_directory_entry_t* dir_entry, emmc_block_t* block){
  bios_parameter_block_t* bpb = &fat.bs16.fields.bpb;
  uint32_t num_blocks = ((dir_entry->file_size + (EMMC_BLOCK_SIZE - 1)) / EMMC_BLOCK_SIZE);
  uint32_t cluster_offset = num_blocks / bpb->sectors_per_cluster;
  uint32_t sector_offset = num_blocks % bpb->sectors_per_cluster;

  SYS_LOGV("num_blocks: %d, dir_entry file_size: %d", num_blocks, dir_entry->file_size);

  // if there are still emtpy blocks in cluster we can simply write to them
  if((num_blocks % bpb->sectors_per_cluster) == 0){
    STATUS_OK_OR_RETURN(fat_append_cluster(dir_entry));
  }

  STATUS_OK_OR_RETURN(fat_write_block(dir_entry, num_blocks, block));
  return STATUS_OK;
}

status_t fat_open_file(const char* file_name, int flags, int* fd){
  if((flags & (O_RDONLY | O_WRONLY | O_RDWR)) == 0){
    SYS_LOGV("invalid flags: %#x, %#x", flags, (O_RDONLY | O_WRONLY | O_RDWR));
    *fd = -1;
    return STATUS_ERR;
  }else if(strlen(file_name) > MAX_FILE_NAME_LEN){
    SYS_LOG("invalid file name: %s", file_name);
    return STATUS_ERR;
  }

  fat_directory_entry_t* dir_entry = NULL;
  for(uint32_t i = 3; i < MAX_OPEN_FILES; ++i){
    if(system_inode_table[i].dir_entry.name[0] == 0){
      SYS_LOGV("found free file descriptor: %d", i);
      system_inode_table[i].flags = flags;
      system_inode_table[i].file_offset = 0;
      strcpy(system_inode_table[i].file_name, file_name);
      SYS_LOGV("file_name: %s", system_inode_table[i].file_name);
      dir_entry = &system_inode_table[i].dir_entry;
      *fd = i;
      break;
    }
  }
  
  if(dir_entry == NULL){
    SYS_LOGV("no free file descriptors found");
    *fd = -1;
    return STATUS_ERR;
  }

  if(fat_get_dir_entry(file_name, dir_entry) == STATUS_OK){
    if(flags & O_APPEND) system_inode_table[*fd].file_offset = dir_entry->file_size;
    return STATUS_OK;
  }

  if((flags & O_CREAT) != 0){
    if(fat_create_file(file_name, dir_entry) != STATUS_OK){
      SYS_LOGE("failed to create file");
      memset(&system_inode_table[*fd].dir_entry, 0, sizeof(fat_directory_entry_t));
      *fd = -1;
      return STATUS_ERR;
    }
    return STATUS_OK;
  }
  SYS_LOGE("file %s doesn't exist and O_CREAT not set", file_name);
  memset(&system_inode_table[*fd].dir_entry, 0, sizeof(fat_directory_entry_t));
  *fd = -1;
  return STATUS_ERR;
}

status_t fat_close_file(int fd){
  if(fd >= MAX_OPEN_FILES){
    return STATUS_ERR;
  }

  STATUS_OK_OR_RETURN(fat_set_dir_entry(system_inode_table[fd].file_name, &system_inode_table[fd].dir_entry));
  memset(&system_inode_table[fd].dir_entry, 0, sizeof(fat_directory_entry_t));
  return STATUS_OK;
}

status_t fat_read_file(int fd, char *buf, int len, int* bytes_read){
  fat_inode_t* inode = &system_inode_table[fd];
  if((inode->flags & (O_RDONLY | O_RDWR)) == 0){
    SYS_LOGE("read failed invalid flags: %#x", inode->flags);
    return STATUS_ERR;
  }else if(len > inode->dir_entry.file_size){
    SYS_LOGE("read length greater than file size: %d > %d", len, inode->dir_entry.file_size);
    return STATUS_ERR;
  }

  uint32_t block_offset = inode->file_offset / EMMC_BLOCK_SIZE;
  uint32_t block_index = (inode->file_offset % EMMC_BLOCK_SIZE);
  uint32_t block_num = len / EMMC_BLOCK_SIZE;
  if(((len % EMMC_BLOCK_SIZE) + block_index) > EMMC_BLOCK_SIZE) block_num++;

  SYS_LOGV("file_offset: %d, block_offset: %d, block_num: %d", inode->file_offset, block_offset, block_num);
  
  uint32_t buf_offset = 0;
  emmc_block_t block;
  for(uint32_t i = 0; i <= block_num; ++i){
    STATUS_OK_OR_RETURN(fat_read_block(&inode->dir_entry, block_offset + i, &block)); 
    if(i == 0){
      uint32_t memcpy_size = len < (EMMC_BLOCK_SIZE - block_index) ? len : EMMC_BLOCK_SIZE - block_index;
      uint8_t* block_buf = (uint8_t*) block.buf;
      memcpy(buf, block_buf + block_index, memcpy_size);
      buf_offset = memcpy_size;
    }else if(i == block_num){
      if(((len % EMMC_BLOCK_SIZE) + block_index) > EMMC_BLOCK_SIZE){
        memcpy(&buf[buf_offset], block.buf, (len % EMMC_BLOCK_SIZE) - (EMMC_BLOCK_SIZE - block_index));
      }else{
        memcpy(&buf[buf_offset], block.buf, (len % EMMC_BLOCK_SIZE) + block_index);
      }
    }else{
      memcpy(&buf[buf_offset], block.buf, EMMC_BLOCK_SIZE);
      buf_offset += EMMC_BLOCK_SIZE;
    }
  }

  inode->file_offset += len;
  *bytes_read = len;
  return STATUS_OK;
}

status_t fat_write_file(int fd, char* buf, int len, int* bytes_written){
  fat_inode_t* inode = &system_inode_table[fd];
  if((inode->flags & (O_WRONLY | O_RDWR)) == 0){
    SYS_LOGE("write failed invalid flags: %#x", inode->flags);
    return STATUS_ERR;
  }
  
  uint32_t block_offset = inode->file_offset / EMMC_BLOCK_SIZE;
  uint32_t block_index = (inode->file_offset % EMMC_BLOCK_SIZE);
  uint32_t block_num = len / EMMC_BLOCK_SIZE;

  if(((len % EMMC_BLOCK_SIZE) + block_index) > EMMC_BLOCK_SIZE) block_num++;

  uint32_t buf_offset = 0;
  emmc_block_t block;

  STATUS_OK_OR_RETURN(fat_read_block(&inode->dir_entry, block_offset, &block)); 
  uint32_t memcpy_size = len < (EMMC_BLOCK_SIZE - block_index) ? len : EMMC_BLOCK_SIZE - block_index;
  uint8_t* block_buf = (uint8_t*) block.buf;
  memcpy(block_buf + block_index, buf, memcpy_size);

  buf_offset = memcpy_size;
  inode->file_offset += memcpy_size;

  if((inode->dir_entry.file_size == 0) || (inode->file_offset <= inode->dir_entry.file_size) || (block_index > 0)){
    SYS_LOGV("block_offset: %d", block_offset);
    STATUS_OK_OR_RETURN(fat_write_block(&inode->dir_entry, block_offset, &block)); 
    if(inode->file_offset > inode->dir_entry.file_size) inode->dir_entry.file_size = inode->file_offset;
  }else{
    STATUS_OK_OR_RETURN(fat_append_block(&inode->dir_entry, &block)); 
    inode->dir_entry.file_size = inode->file_offset;
  }

  for(uint32_t i = 1; i <= block_num; ++i){
    if(i == block_num){
      memset(block.buf, 0, EMMC_BLOCK_SIZE);
      if(inode->file_offset < inode->file_offset){ 
        STATUS_OK_OR_RETURN(fat_read_block(&inode->dir_entry, block_offset + i, &block)); 
      }
      if(((len % EMMC_BLOCK_SIZE) + block_index) > EMMC_BLOCK_SIZE){
        memcpy(block.buf, &buf[buf_offset], (len % EMMC_BLOCK_SIZE) - (EMMC_BLOCK_SIZE - block_index));
        inode->file_offset += ((len % EMMC_BLOCK_SIZE) + block_index) - (EMMC_BLOCK_SIZE);
      }else{
        memcpy(block.buf, &buf[buf_offset], (len % EMMC_BLOCK_SIZE) + block_index);
        inode->file_offset += (len % EMMC_BLOCK_SIZE) + block_index;
      }
    }else{
      memcpy(block.buf, &buf[buf_offset], EMMC_BLOCK_SIZE);
      buf_offset += EMMC_BLOCK_SIZE;
      inode->file_offset += EMMC_BLOCK_SIZE;
    }

    if(inode->file_offset < inode->dir_entry.file_size){ 
      STATUS_OK_OR_RETURN(fat_write_block(&inode->dir_entry, block_offset + i, &block));
    }else{
      STATUS_OK_OR_RETURN(fat_append_block(&inode->dir_entry, &block));
      inode->dir_entry.file_size = inode->file_offset;
    }
  }

  return STATUS_OK;
}

