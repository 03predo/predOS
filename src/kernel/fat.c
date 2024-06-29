#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

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

#define MAX_FILE_NAME_LEN 12

static file_allocation_table_t fat;

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

  SYS_LOGD("write complete");
  fat.partition_base_sector = fat.mbr.fields.p1.lba_first_sector_address;
  emmc_read_block(fat.partition_base_sector, 1, &block);

  boot_sector_fat32_t bs32;
  memcpy(&fat.bs16.raw[0], block.buf, FAT16_BOOT_SECTOR_SIZE);
  memcpy(&fat.bs32.raw[0], block.buf, FAT32_BOOT_SECTOR_SIZE);

  bios_parameter_block_t* bpb = &fat.bs32.fields.bpb;
  if((bpb->jump_boot[0] != VALID_JUMP_BOOT0) || (bpb->jump_boot[2] != VALID_JUMP_BOOT2)){
    SYS_LOGE("invalid jump_boot: %#x %#x %#x", bpb->jump_boot[0], bpb->jump_boot[1], bpb->jump_boot[2]);
    sys_timer_sleep(100000);
    for(uint32_t i = 0; i < FAT32_BOOT_SECTOR_SIZE; ++i){
      printf("%d: %#x\n", i, fat.bs32.raw[i]);
    }
    sys_timer_sleep(100000);
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
    /*
    fat.type = FAT32;
    fat.fat_base_sector = bpb->reserved_sector_count;
    fat.root_dir_base_sector = fat.bs32.fields.root_cluster * bpb->sectors_per_cluster;
    */
  }
  SYS_LOGV("fat init done");

  fat_print_bpb();
  return STATUS_OK;
}

status_t fat_convert_file_name(const char* file_name, fat_directory_entry_t* dir_entry){
  const char dot[2] = ".";
  char file_name_copy[MAX_FILE_NAME_LEN];
  strcpy(file_name_copy, file_name);
  char* main_name = strtok(file_name_copy, dot);
  char* extension = strtok(NULL, dot);

  if(strtok(NULL, dot) != NULL){
    SYS_LOGE("invalid file path: %s", file_name);
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
  if(strlen(file_name) > MAX_FILE_NAME_LEN){
    SYS_LOGE("file name is too long: %s", file_name);
    return STATUS_ERR;
  }

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

status_t fat_find_free_cluster(fat_directory_entry_t* dir_entry){
  bios_parameter_block_t bpb = fat.bs16.fields.bpb;
  emmc_block_t block;
  for(uint32_t i = 0; i < bpb.fat_sector_count_16bit; ++i){
    emmc_read_block(fat.partition_base_sector + fat.fat_base_sector + i, 1, &block);
    uint16_t* fat_block = (uint16_t*)block.buf;
    for(uint32_t j = 0; j < (EMMC_BLOCK_SIZE / sizeof(uint16_t)); ++j){
      if(fat_block[j] == 0x0){
        dir_entry->first_cluster_low = j + i * (EMMC_BLOCK_SIZE / sizeof(uint16_t));
        SYS_LOGV("free cluster: %d", dir_entry->first_cluster_low);
        fat_block[j] = 0xffff;
        emmc_write_block(fat.partition_base_sector + fat.fat_base_sector + i, 1, &block);
        emmc_print_block(block);
        return STATUS_OK;
      }
    }
  }

  return STATUS_ERR;
}

status_t fat_create_file(const char* file_name){
  SYS_LOGV("creating file %s", file_name);
  fat_directory_entry_t dir_entry = {0};
  STATUS_OK_OR_RETURN(fat_convert_file_name(file_name, &dir_entry));


  dir_entry.file_size = 32;
  dir_entry.attribute = 0x20;
  dir_entry.creation_date.day = 1;
  dir_entry.creation_date.month = 1;
  dir_entry.creation_date.year = 1;
  dir_entry.creation_time.two_seconds = 1;
  dir_entry.creation_time.minutes = 1;
  dir_entry.creation_time.hours = 1;

  dir_entry.last_access_date = dir_entry.creation_date;
  dir_entry.last_write_date = dir_entry.creation_date;
  dir_entry.last_write_time = dir_entry.creation_time;
  dir_entry.reserved0 = 0x18;

  bios_parameter_block_t bpb = fat.bs16.fields.bpb;
  uint32_t root_dir_sector_count = ((bpb.root_entry_count * sizeof(fat_directory_entry_t)) + (bpb.bytes_per_sector - 1)) / bpb.bytes_per_sector; 

  STATUS_OK_OR_RETURN(fat_find_free_cluster(&dir_entry));

  // find free directory entry
  fat_directory_entry_t search_dir_entry = {0};
  emmc_block_t block;
  for(uint32_t i = 0; i < root_dir_sector_count; ++i){
    emmc_read_block(fat.partition_base_sector + fat.root_dir_base_sector + i, 1, &block);
    uint8_t* block_buf = (uint8_t*)block.buf;
    for(uint32_t j = 0; j < (EMMC_BLOCK_SIZE / sizeof(fat_directory_entry_t)); ++j){
      memcpy(&search_dir_entry, block_buf + (j * (sizeof(fat_directory_entry_t))), sizeof(fat_directory_entry_t));
      if(search_dir_entry.name[0] == 0x00 || search_dir_entry.name[0] == 0xe5){
        memcpy(block_buf + (j * (sizeof(fat_directory_entry_t))), &dir_entry, sizeof(fat_directory_entry_t));
        fat_print_entry(dir_entry);
        emmc_write_block(fat.partition_base_sector + fat.root_dir_base_sector + i, 1, &block);
        return STATUS_OK;
      }
    }
  } 
  return STATUS_ERR;
}

status_t fat_get_absolute_cluster(fat_directory_entry_t* dir_entry, uint32_t file_cluster_offset, uint32_t* absolute_cluster){
  // the first cluster of the file is also the offset into the FAT
  uint32_t file_first_cluster = dir_entry->first_cluster_low + (dir_entry->first_cluster_high << 16);

  emmc_block_t block;
  uint16_t* fat_block;
  uint32_t fat_sector_offset = 0;
  uint32_t fat_entry_offset = 0;
  *absolute_cluster = file_first_cluster;
  if(fat.type == FAT16){ 
    for(uint32_t i = 0; i < file_cluster_offset; ++i){
      fat_sector_offset = *absolute_cluster / (EMMC_BLOCK_SIZE / sizeof(uint16_t));
      fat_entry_offset = *absolute_cluster  % (EMMC_BLOCK_SIZE / sizeof(uint16_t));
      emmc_read_block(fat.partition_base_sector + fat.fat_base_sector + fat_sector_offset, 1, &block);
      fat_block = (uint16_t*)block.buf;
      *absolute_cluster = fat_block[fat_entry_offset];
      SYS_LOGV("fat_entry_offset: %#x, absolute_cluster: %#x", fat_entry_offset, *absolute_cluster);
    }
  }else{
    SYS_LOGE("unsupported fat type");
    return STATUS_ERR;
  }
  
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

