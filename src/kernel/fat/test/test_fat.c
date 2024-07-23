#include <stdlib.h>
#include <unity.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

#include "fat.h"

#include "mock_sys_timer.h"
#include "mock_emmc.h"

#define BYTE0(X) (0xff & X)
#define BYTE1(X) ((0xff00 & X) >> 8)
#define BYTE2(X) ((0xff0000 & X) >> 16)
#define BYTE3(X) ((0xff000000 & X) >> 24)

#define PARTITION_BASE_SEC 0x800
#define FAT_BASE_SEC 0x4

status_t fat_validate_file_name(const char* file_name);
status_t fat_convert_file_name(const char* file_name, fat_directory_entry_t* dir_entry);
status_t fat_get_dir_entry(const char* file_name, fat_directory_entry_t* dir_entry);
status_t fat_set_dir_entry(const char* file_name, fat_directory_entry_t* dir_entry);
status_t fat_find_free_cluster(uint32_t* cluster);
status_t fat_get_absolute_cluster(fat_directory_entry_t* dir_entry, uint32_t file_cluster_offset, uint32_t* absolute_cluster);
status_t fat_read_file(int fd, char *buf, int len, int* bytes_read);
status_t fat_free_cluster_table();
status_t fat_read_cluster_table();

uint32_t* disk = NULL;
boot_sector_fat32_t* bs32 = NULL;

void setUp (void) {}
void tearDown (void) {}

status_t mock_emmc_init(){
  const char img_file[] = IMG_DIR"/predOS.img";
  printf("img_file: %s\n", img_file);

  struct stat st;
  stat(img_file, &st);
  printf("file size: %ld\n", st.st_size);

  if(disk != NULL) free(disk);
  disk = calloc(st.st_size / sizeof(uint32_t), sizeof(uint32_t));
  
  if(disk == NULL){
    printf("failed to allocate buf");
    return STATUS_ERR;
  }
  int fd = open(IMG_DIR"/predOS.img", O_RDWR);
  if(fd == -1){
    printf("failed to open file (%d)\n", errno);
    return STATUS_ERR;
  }

  int bytes_read = read(fd, disk, st.st_size);
  printf("bytes_read: %d\n", bytes_read);
  return STATUS_OK;
}

status_t mock_emmc_init_stub0(int cmock_num_calls){
  return mock_emmc_init();
}

status_t mock_emmc_read_block(uint32_t start_block_address, uint16_t num_blocks, emmc_block_t* block){
  uint32_t* start_block = &disk[start_block_address * (EMMC_BLOCK_SIZE / sizeof(uint32_t))];
  for(uint32_t i = 0; i < num_blocks * (EMMC_BLOCK_SIZE / sizeof(uint32_t)); ++i){
      block[i / (EMMC_BLOCK_SIZE / sizeof(uint32_t))].buf[i % (EMMC_BLOCK_SIZE / sizeof(uint32_t))] = start_block[i];
  }
  return STATUS_OK;
}

status_t mock_emmc_read_block_stub0(uint32_t start_block_address, uint16_t num_blocks, emmc_block_t* block, int cmock_num_calls){
  return mock_emmc_read_block(start_block_address, num_blocks, block);
}

status_t mock_emmc_read_block_stub1(uint32_t start_block_address, uint16_t num_blocks, emmc_block_t* block, int cmock_num_calls){
  for(uint32_t i = 0; i < (EMMC_BLOCK_SIZE / sizeof(uint32_t)); ++i){
    block->buf[i] = 0;
  }
  return STATUS_OK;
}

status_t mock_emmc_read_block_stub2(uint32_t start_block_address, uint16_t num_blocks, emmc_block_t* block, int cmock_num_calls){
  printf("cmock_num_calls: %d\n", cmock_num_calls);
  switch(cmock_num_calls){
    case 0:
      return mock_emmc_read_block(start_block_address, num_blocks, block);

    case 1:
      for(uint32_t i = 0; i < (EMMC_BLOCK_SIZE / sizeof(uint32_t)); ++i){
        block->buf[i] = 0;
      }
      return STATUS_OK;

    default:
      return STATUS_ERR;
  }
  return STATUS_ERR;
}

status_t mock_emmc_read_block_stub3(uint32_t start_block_address, uint16_t num_blocks, emmc_block_t* block, int cmock_num_calls){
  switch(cmock_num_calls){
    case 0:
      return mock_emmc_read_block(start_block_address, num_blocks, block);

    case 1:
      mock_emmc_read_block(start_block_address, num_blocks, block);
      bios_parameter_block_t* bpb = (bios_parameter_block_t*) block;
      bpb->fat_sector_count_16bit = 0;
      return STATUS_OK;

    default:
      return STATUS_ERR;
  } 
  return STATUS_ERR;
}

status_t mock_emmc_read_block_stub4(uint32_t start_block_address, uint16_t num_blocks, emmc_block_t* block, int cmock_num_calls){
  switch(cmock_num_calls){
    case 0:
      return mock_emmc_read_block(start_block_address, num_blocks, block);

    case 1:
      mock_emmc_read_block(start_block_address, num_blocks, block);
      bs32 = (boot_sector_fat32_t*) block;
      bs32->fields.bpb.fat_sector_count_16bit = 0;
      bs32->fields.fat_sector_count_32bit = 0;
      return STATUS_OK;

    default:
      return STATUS_ERR;
  } 
  return STATUS_ERR;  
}

status_t mock_emmc_read_block_stub5(uint32_t start_block_address, uint16_t num_blocks, emmc_block_t* block, int cmock_num_calls){
  switch(cmock_num_calls){
    case 0:
      return mock_emmc_read_block(start_block_address, num_blocks, block);

    case 1:
      mock_emmc_read_block(start_block_address, num_blocks, block);
      bs32 = (boot_sector_fat32_t*) block;
      bs32->fields.bpb.total_sector_count_16bit = 0;
      bs32->fields.bpb.total_sector_count_32bit = 1;
      bs32->fields.bpb.reserved_sector_count = 0;
      bs32->fields.bpb.number_of_fats = 0;
      bs32->fields.bpb.fat_sector_count_16bit = 0;
      bs32->fields.bpb.bytes_per_sector = 1;
      bs32->fields.bpb.root_entry_count = 0;
      return STATUS_OK;

    default:
      return STATUS_ERR;
  } 
  return STATUS_ERR;  
}

status_t mock_emmc_read_block_stub6(uint32_t start_block_address, uint16_t num_blocks, emmc_block_t* block, int cmock_num_calls){
  switch(cmock_num_calls){
    case 0:
      return mock_emmc_read_block(start_block_address, num_blocks, block);

    case 1:
      mock_emmc_read_block(start_block_address, num_blocks, block);
      bs32 = (boot_sector_fat32_t*) block;
      bs32->fields.bpb.total_sector_count_16bit = 0;
      bs32->fields.bpb.total_sector_count_32bit = 0;
      return STATUS_OK;

    default:
      return STATUS_ERR;
  } 
  return STATUS_ERR;  
}

status_t mock_emmc_read_block_stub7(uint32_t start_block_address, uint16_t num_blocks, emmc_block_t* block, int cmock_num_calls){
  switch(cmock_num_calls){
    case 1:
      mock_emmc_read_block(start_block_address, num_blocks, block);
      bs32 = (boot_sector_fat32_t*) block->buf;
      return STATUS_OK;

    default:
      return mock_emmc_read_block(start_block_address, num_blocks, block);
  } 
  return STATUS_ERR;  
}

status_t mock_emmc_write_block(uint32_t start_block_address, uint16_t num_blocks, emmc_block_t* block){
  uint32_t* start_block = &disk[start_block_address * (EMMC_BLOCK_SIZE / sizeof(uint32_t))];
  for(uint32_t i = 0; i < num_blocks * (EMMC_BLOCK_SIZE / sizeof(uint32_t)); ++i){
    start_block[i] = block[i / (EMMC_BLOCK_SIZE / sizeof(uint32_t))].buf[i % EMMC_BLOCK_SIZE];
  }
  return STATUS_OK;
}

status_t mock_emmc_write_block_stub0(uint32_t start_block_address, uint16_t num_blocks, emmc_block_t* block, int cmock_num_calls){
  return mock_emmc_write_block(start_block_address, num_blocks, block);
}

void test_fat_print_entry(){
  sys_uptime_IgnoreAndReturn(0);

  fat_directory_entry_t dir_entry;
  memset(&dir_entry, 0, sizeof(fat_directory_entry_t));
  TEST_ASSERT_EQUAL(STATUS_OK, fat_print_entry(dir_entry));
}

void test_fat_init(){
  sys_uptime_IgnoreAndReturn(0);
  sys_timer_sleep_Ignore();
 
  mock_emmc_Init();
  emmc_init_Stub(mock_emmc_init_stub0);
  emmc_read_block_Stub(mock_emmc_read_block_stub0); 
  TEST_ASSERT_EQUAL(STATUS_OK, fat_init());
  TEST_ASSERT_EQUAL(STATUS_OK, fat_free_cluster_table());
  
  mock_emmc_Init();
  emmc_init_Stub(mock_emmc_init_stub0);
  emmc_read_block_Stub(mock_emmc_read_block_stub1);
  TEST_ASSERT_EQUAL(STATUS_ERR, fat_init());

  mock_emmc_Init();
  emmc_init_Stub(mock_emmc_init_stub0);
  emmc_read_block_Stub(mock_emmc_read_block_stub2);
  TEST_ASSERT_EQUAL(STATUS_ERR, fat_init());

  mock_emmc_Init();
  emmc_init_Stub(mock_emmc_init_stub0);
  emmc_read_block_Stub(mock_emmc_read_block_stub3);
  TEST_ASSERT_EQUAL(STATUS_ERR, fat_init());

  mock_emmc_Init();
  emmc_init_Stub(mock_emmc_init_stub0);
  emmc_read_block_Stub(mock_emmc_read_block_stub4);
  TEST_ASSERT_EQUAL(STATUS_ERR, fat_init());

  mock_emmc_Init();
  emmc_init_Stub(mock_emmc_init_stub0);
  emmc_read_block_Stub(mock_emmc_read_block_stub5);
  TEST_ASSERT_EQUAL(STATUS_ERR, fat_init());
 
  mock_emmc_Init();
  emmc_init_Stub(mock_emmc_init_stub0);
  emmc_read_block_Stub(mock_emmc_read_block_stub6);
  TEST_ASSERT_EQUAL(STATUS_ERR, fat_init());
}

void test_fat_validate_file_name(){
  sys_uptime_IgnoreAndReturn(0); 
  TEST_ASSERT_EQUAL(STATUS_OK, fat_validate_file_name("\5log.txt"));
  TEST_ASSERT_EQUAL(STATUS_ERR, fat_validate_file_name(".txt"));
  TEST_ASSERT_EQUAL(STATUS_ERR, fat_validate_file_name(" txt"));
  char file_name[] = "log.txt";
  file_name[0] = 0x22;
  TEST_ASSERT_EQUAL(STATUS_ERR, fat_validate_file_name(file_name));
  file_name[0] = 'l';
  file_name[1] = 0x22;
  TEST_ASSERT_EQUAL(STATUS_ERR, fat_validate_file_name(file_name));
  file_name[1] = ' ' - 1;
  TEST_ASSERT_EQUAL(STATUS_ERR, fat_validate_file_name(file_name));
}

void test_fat_convert_file_name(){
  sys_uptime_IgnoreAndReturn(0);

  fat_directory_entry_t dir_entry;
  TEST_ASSERT_EQUAL(STATUS_OK, fat_convert_file_name("log.txt", &dir_entry));
  TEST_ASSERT_EQUAL_CHAR_ARRAY("LOG     TXT", dir_entry.name, FAT_DIR_ENTRY_NAME_LEN);

  TEST_ASSERT_EQUAL(STATUS_ERR, fat_convert_file_name("123456789.txt", &dir_entry));
  TEST_ASSERT_EQUAL(STATUS_ERR, fat_convert_file_name("123456789", &dir_entry));
  TEST_ASSERT_EQUAL(STATUS_ERR, fat_convert_file_name("1.1234", &dir_entry));
  TEST_ASSERT_EQUAL(STATUS_ERR, fat_convert_file_name("log.1.txt", &dir_entry));
  TEST_ASSERT_EQUAL(STATUS_OK, fat_convert_file_name("log.tx", &dir_entry));
}

void test_fat_get_dir_entry(){
  sys_uptime_IgnoreAndReturn(0);
  
  fat_directory_entry_t dir_entry;

  emmc_init_Stub(mock_emmc_init_stub0);
  emmc_read_block_Stub(mock_emmc_read_block_stub0); 

  TEST_ASSERT_EQUAL(STATUS_OK, fat_init());
  TEST_ASSERT_EQUAL(STATUS_OK, fat_get_dir_entry("config.txt", &dir_entry));
  TEST_ASSERT_EQUAL_CHAR_ARRAY("CONFIG  TXT", dir_entry.name, FAT_DIR_ENTRY_NAME_LEN);
  TEST_ASSERT_EQUAL(STATUS_OK, fat_free_cluster_table());

  TEST_ASSERT_EQUAL(STATUS_OK, fat_init());
  TEST_ASSERT_EQUAL(STATUS_ERR, fat_get_dir_entry("config_.txt", &dir_entry));
  TEST_ASSERT_EQUAL(STATUS_OK, fat_free_cluster_table());
}

void test_fat_set_dir_entry(){
  sys_uptime_IgnoreAndReturn(0);
  emmc_init_Stub(mock_emmc_init_stub0);
  emmc_read_block_Stub(mock_emmc_read_block_stub0); 
  emmc_write_block_Stub(mock_emmc_write_block_stub0); 

  fat_directory_entry_t dir_entry;
  TEST_ASSERT_EQUAL(STATUS_OK, fat_init());
  TEST_ASSERT_EQUAL(STATUS_OK, fat_get_dir_entry("config.txt", &dir_entry));
  memcpy(dir_entry.name, "CONFIG1 TXT", FAT_DIR_ENTRY_NAME_LEN);
  printf("c: %c\n", dir_entry.name[6]);
  TEST_ASSERT_EQUAL(STATUS_OK, fat_set_dir_entry("config.txt", &dir_entry));
  TEST_ASSERT_EQUAL(STATUS_OK, fat_get_dir_entry("config1.txt", &dir_entry));
  TEST_ASSERT_EQUAL_CHAR_ARRAY("CONFIG1 TXT", dir_entry.name, FAT_DIR_ENTRY_NAME_LEN);
  TEST_ASSERT_EQUAL(STATUS_ERR, fat_set_dir_entry("config.txt", &dir_entry));
  TEST_ASSERT_EQUAL(STATUS_OK, fat_free_cluster_table());
}

void test_fat_find_free_cluster(){
  sys_uptime_IgnoreAndReturn(0);
  emmc_init_Stub(mock_emmc_init_stub0);
  emmc_read_block_Stub(mock_emmc_read_block_stub0); 
  emmc_write_block_Stub(mock_emmc_write_block_stub0); 

  TEST_ASSERT_EQUAL(STATUS_OK, fat_init());

  uint32_t cluster = 0;
  TEST_ASSERT_EQUAL(STATUS_OK, fat_find_free_cluster(&cluster));
  TEST_ASSERT_NOT_EQUAL(0, cluster);
  TEST_ASSERT_EQUAL(STATUS_OK, fat_free_cluster_table());

  mock_emmc_Init();
  emmc_init_Stub(mock_emmc_init_stub0);
  emmc_read_block_Stub(mock_emmc_read_block_stub7);
  emmc_write_block_Stub(mock_emmc_write_block_stub0); 

  TEST_ASSERT_EQUAL(STATUS_OK, fat_init());
  printf("%#x, %#x, %#x\n", bs32->fields.bpb.root_entry_count, bs32->fields.bpb.bytes_per_sector, bs32->fields.bpb.fat_sector_count_16bit);
  emmc_block_t block;
  memset(&block, 0xff, sizeof(emmc_block_t));
  for(uint32_t i = 0; i < bs32->fields.bpb.fat_sector_count_16bit; ++i){
    mock_emmc_write_block(PARTITION_BASE_SEC + FAT_BASE_SEC + i, 1, &block);
  }
  TEST_ASSERT_EQUAL(STATUS_OK, fat_free_cluster_table());
  TEST_ASSERT_EQUAL(STATUS_OK, fat_read_cluster_table()); 

  TEST_ASSERT_EQUAL(STATUS_ERR, fat_find_free_cluster(&cluster));
  TEST_ASSERT_EQUAL(STATUS_OK, fat_free_cluster_table());
}

void test_fat_create_file(){
  sys_uptime_IgnoreAndReturn(0);
  emmc_init_Stub(mock_emmc_init_stub0);
  emmc_read_block_Stub(mock_emmc_read_block_stub7);
  emmc_write_block_Stub(mock_emmc_write_block_stub0); 

  fat_directory_entry_t dir_entry;
  TEST_ASSERT_EQUAL(STATUS_OK, fat_init());
  TEST_ASSERT_EQUAL(STATUS_OK, fat_create_file("tmp.txt", &dir_entry));
  TEST_ASSERT_EQUAL(STATUS_OK, fat_get_dir_entry("tmp.txt", &dir_entry));
  TEST_ASSERT_EQUAL_CHAR_ARRAY("TMP     TXT", dir_entry.name, FAT_DIR_ENTRY_NAME_LEN);

  mock_emmc_Init();
  emmc_init_Stub(mock_emmc_init_stub0);
  emmc_read_block_Stub(mock_emmc_read_block_stub7);
  emmc_write_block_Stub(mock_emmc_write_block_stub0); 
  TEST_ASSERT_EQUAL(STATUS_OK, fat_init());

  printf("%#x, %#x\n", bs32->fields.bpb.root_entry_count, bs32->fields.bpb.bytes_per_sector);
  bios_parameter_block_t* bpb = &bs32->fields.bpb;
  uint32_t root_dir_sector_count = ((bpb->root_entry_count * 32) + (bpb->bytes_per_sector - 1)) / bpb->bytes_per_sector;
  uint32_t root_dir_base_sector = bpb->reserved_sector_count + (bpb->number_of_fats * bpb->fat_sector_count_16bit);
  printf("sec count: %d, base sec: %d", root_dir_sector_count, root_dir_base_sector);
  emmc_block_t block;
  for(uint32_t i = 0; i < root_dir_sector_count; ++i){
    mock_emmc_read_block(PARTITION_BASE_SEC + root_dir_base_sector + i, 1, &block);
    uint8_t* block_buf = (uint8_t*)block.buf;
    for(uint32_t j = 0; j < (EMMC_BLOCK_SIZE / sizeof(fat_directory_entry_t)); ++j){
      fat_directory_entry_t* search_dir_entry = (fat_directory_entry_t*) (block_buf + (j*(sizeof(fat_directory_entry_t))));
      search_dir_entry->name[0] = 'a';
    }
    mock_emmc_write_block(PARTITION_BASE_SEC + root_dir_base_sector + i, 1, &block);
  }

  TEST_ASSERT_EQUAL(STATUS_ERR, fat_create_file("tmp1.txt", &dir_entry));
  TEST_ASSERT_EQUAL(STATUS_OK, fat_free_cluster_table());
}

void test_fat_get_absolute_cluster(){ 
  sys_uptime_IgnoreAndReturn(0);
  emmc_init_Stub(mock_emmc_init_stub0);
  emmc_read_block_Stub(mock_emmc_read_block_stub7);

  fat_directory_entry_t dir_entry;
  uint32_t absolute_cluster = 0;
  TEST_ASSERT_EQUAL(STATUS_OK, fat_init());
  TEST_ASSERT_EQUAL(STATUS_OK, fat_get_dir_entry("kernel.img", &dir_entry));
  TEST_ASSERT_EQUAL(STATUS_OK, fat_get_absolute_cluster(&dir_entry, 4, &absolute_cluster));

  TEST_ASSERT_EQUAL(STATUS_OK, fat_get_dir_entry("config.txt", &dir_entry));
  TEST_ASSERT_EQUAL(STATUS_ERR, fat_get_absolute_cluster(&dir_entry, 4, &absolute_cluster));
  TEST_ASSERT_NOT_EQUAL(0, absolute_cluster);

  TEST_ASSERT_EQUAL(STATUS_OK, fat_free_cluster_table());
}

void test_fat_read_block(){
  sys_uptime_IgnoreAndReturn(0);
  emmc_init_Stub(mock_emmc_init_stub0);
  emmc_read_block_Stub(mock_emmc_read_block_stub7);

  fat_directory_entry_t dir_entry;
  emmc_block_t block;
  TEST_ASSERT_EQUAL(STATUS_OK, fat_init());
  TEST_ASSERT_EQUAL(STATUS_OK, fat_get_dir_entry("config.txt", &dir_entry));
  TEST_ASSERT_EQUAL(STATUS_OK, fat_read_block(&dir_entry, 0, 1, &block));
  TEST_ASSERT_EQUAL_CHAR_ARRAY("start_file", block.buf, 10);

  TEST_ASSERT_EQUAL(STATUS_OK, fat_free_cluster_table());
}

void test_fat_write_block(){
  sys_uptime_IgnoreAndReturn(0);
  emmc_init_Stub(mock_emmc_init_stub0);
  emmc_read_block_Stub(mock_emmc_read_block_stub7);
  emmc_write_block_Stub(mock_emmc_write_block_stub0);

  fat_directory_entry_t dir_entry;
  emmc_block_t block;
  memcpy(block.buf, "write", 5);
  TEST_ASSERT_EQUAL(STATUS_OK, fat_init());
  TEST_ASSERT_EQUAL(STATUS_OK, fat_get_dir_entry("config.txt", &dir_entry));
  TEST_ASSERT_EQUAL(STATUS_OK, fat_write_block(&dir_entry, 0, &block));
  TEST_ASSERT_EQUAL(STATUS_OK, fat_read_block(&dir_entry, 0, 1, &block));
  TEST_ASSERT_EQUAL_CHAR_ARRAY("write", block.buf, 5);

  TEST_ASSERT_EQUAL(STATUS_OK, fat_free_cluster_table());
}

void test_fat_open_file(){
  sys_uptime_IgnoreAndReturn(0);
  emmc_init_Stub(mock_emmc_init_stub0);
  emmc_read_block_Stub(mock_emmc_read_block_stub7);
  emmc_write_block_Stub(mock_emmc_write_block_stub0);

  int fd = -1;
  TEST_ASSERT_EQUAL(STATUS_OK, fat_init());
  for(uint32_t i = 0; i < MAX_OPEN_FILES - 3; ++i){
    TEST_ASSERT_EQUAL(STATUS_OK, fat_open_file("config.txt", O_RDWR, &fd));
    TEST_ASSERT_NOT_EQUAL(-1, fd);
  } 

  TEST_ASSERT_EQUAL(STATUS_ERR, fat_open_file("config.txt", O_RDWR, &fd)); 
  TEST_ASSERT_EQUAL(-1, fd);

  TEST_ASSERT_EQUAL(STATUS_OK, fat_free_cluster_table());
  TEST_ASSERT_EQUAL(STATUS_OK, fat_init());
  TEST_ASSERT_EQUAL(STATUS_ERR, fat_open_file("config.txt", O_RDWR | O_WRONLY, &fd)); 
  TEST_ASSERT_EQUAL(-1, fd);

  TEST_ASSERT_EQUAL(STATUS_ERR, fat_open_file("tmp.txt", O_RDWR, &fd)); 
  TEST_ASSERT_EQUAL(-1, fd);

  TEST_ASSERT_EQUAL(STATUS_OK, fat_open_file("tmp.txt", O_CREAT | O_RDWR, &fd)); 
  TEST_ASSERT_NOT_EQUAL(-1, fd);

  mock_emmc_Init();
  emmc_read_block_Stub(mock_emmc_read_block_stub7);
  emmc_write_block_ExpectAnyArgsAndReturn(STATUS_ERR);

  TEST_ASSERT_EQUAL(STATUS_ERR, fat_open_file("tmp1.txt", O_CREAT | O_RDWR, &fd)); 
  TEST_ASSERT_EQUAL(-1, fd);

  TEST_ASSERT_EQUAL(STATUS_ERR, fat_open_file("123456789.txt", O_CREAT | O_RDWR, &fd)); 
  TEST_ASSERT_EQUAL(-1, fd);
  TEST_ASSERT_EQUAL(STATUS_OK, fat_free_cluster_table());
}

void test_fat_close_file(){
  sys_uptime_IgnoreAndReturn(0);
  emmc_init_Stub(mock_emmc_init_stub0);
  emmc_read_block_Stub(mock_emmc_read_block_stub7);
  emmc_write_block_Stub(mock_emmc_write_block_stub0);

  int fd = -1;
  TEST_ASSERT_EQUAL(STATUS_OK, fat_init());
  TEST_ASSERT_EQUAL(STATUS_OK, fat_open_file("tmp.txt", O_CREAT | O_RDWR, &fd)); 
  TEST_ASSERT_NOT_EQUAL(-1, fd);
  TEST_ASSERT_EQUAL(STATUS_OK, fat_close_file(fd));

  TEST_ASSERT_EQUAL(STATUS_ERR, fat_close_file(MAX_OPEN_FILES));
  TEST_ASSERT_EQUAL(STATUS_OK, fat_free_cluster_table());
}

void test_fat_read_file(){
  sys_uptime_IgnoreAndReturn(0);
  emmc_init_Stub(mock_emmc_init_stub0);
  emmc_read_block_Stub(mock_emmc_read_block_stub7);
  emmc_write_block_Stub(mock_emmc_write_block_stub0);

  printf("filename: %s\n", BUILD_DIR"/src/kernel/kernel.img");
  int img_fd = open(BUILD_DIR"/src/kernel/kernel.img", O_RDWR);
  TEST_ASSERT_NOT_EQUAL(-1, img_fd);

  int fd = -1;
  TEST_ASSERT_EQUAL(STATUS_OK, fat_init());
  TEST_ASSERT_EQUAL(STATUS_OK, fat_open_file("kernel.img", O_CREAT | O_RDWR, &fd)); 
  int bytes_read = 0;
  char buf[1300];
  char img_buf[1300];
  TEST_ASSERT_EQUAL(STATUS_OK, fat_read_file(fd, buf, 1300, &bytes_read));
  TEST_ASSERT_NOT_EQUAL(-1, read(img_fd, img_buf, 1300));
  TEST_ASSERT_EQUAL_UINT8_ARRAY(img_buf, buf, 1300);
  TEST_ASSERT_EQUAL(STATUS_OK, fat_read_file(fd, buf, 511, &bytes_read));
  TEST_ASSERT_NOT_EQUAL(-1, read(img_fd, img_buf, 511));
  TEST_ASSERT_EQUAL_UINT8_ARRAY(img_buf, buf, 511);

  TEST_ASSERT_EQUAL(STATUS_OK, fat_open_file("tmp.txt", O_CREAT | O_RDWR, &fd)); 
  TEST_ASSERT_EQUAL(STATUS_ERR, fat_read_file(fd, buf, 512, &bytes_read));

  TEST_ASSERT_EQUAL(STATUS_OK, fat_open_file("config.txt", O_WRONLY, &fd)); 
  TEST_ASSERT_EQUAL(STATUS_ERR, fat_read_file(fd, buf, 1, &bytes_read));
  TEST_ASSERT_EQUAL(STATUS_OK, fat_free_cluster_table());
}

void test_fat_write_file(){
  sys_uptime_IgnoreAndReturn(0);
  emmc_init_Stub(mock_emmc_init_stub0);
  emmc_read_block_Stub(mock_emmc_read_block_stub7);
  emmc_write_block_Stub(mock_emmc_write_block_stub0);

  int fd = -1;
  TEST_ASSERT_EQUAL(STATUS_OK, fat_init());
  TEST_ASSERT_EQUAL(STATUS_OK, fat_open_file("tmp.txt", O_CREAT | O_WRONLY | O_APPEND, &fd));
  int bytes_written = 0;
  char buf[2200];
  TEST_ASSERT_EQUAL(STATUS_OK, fat_write_file(fd, buf, 2200, &bytes_written));
  TEST_ASSERT_EQUAL(STATUS_OK, fat_write_file(fd, buf, 360, &bytes_written));
  TEST_ASSERT_EQUAL(STATUS_OK, fat_write_file(fd, buf, 10, &bytes_written));
  TEST_ASSERT_NOT_EQUAL(-1, fat_close_file(fd));

  TEST_ASSERT_EQUAL(STATUS_OK, fat_open_file("tmp.txt", O_WRONLY, &fd)); 
  TEST_ASSERT_EQUAL(STATUS_OK, fat_write_file(fd, buf, 10, &bytes_written));
  printf("before second write\n");
  TEST_ASSERT_EQUAL(STATUS_OK, fat_write_file(fd, buf, 511, &bytes_written));
  printf("after second write\n");

  TEST_ASSERT_EQUAL(STATUS_OK, fat_open_file("tmp.txt", O_WRONLY | O_APPEND, &fd)); 
  TEST_ASSERT_EQUAL(STATUS_OK, fat_write_file(fd, buf, 2200, &bytes_written));

  TEST_ASSERT_EQUAL(STATUS_OK, fat_open_file("tmp.txt", O_RDONLY, &fd)); 
  TEST_ASSERT_EQUAL(STATUS_ERR, fat_write_file(fd, buf, 10, &bytes_written));

  TEST_ASSERT_EQUAL(STATUS_OK, fat_free_cluster_table());
}

void test_fat_seek_file(){
  sys_uptime_IgnoreAndReturn(0);
  emmc_init_Stub(mock_emmc_init_stub0);
  emmc_read_block_Stub(mock_emmc_read_block_stub7);
  emmc_write_block_Stub(mock_emmc_write_block_stub0);

  int fd = -1;
  TEST_ASSERT_EQUAL(STATUS_OK, fat_init());
  TEST_ASSERT_EQUAL(STATUS_OK, fat_open_file("config.txt", O_CREAT | O_WRONLY | O_APPEND, &fd)); 
  int new_offset = -1;
  TEST_ASSERT_EQUAL(STATUS_OK, fat_seek_file(fd, 1, SEEK_SET, &new_offset));
  TEST_ASSERT_EQUAL(new_offset, 1);
  TEST_ASSERT_EQUAL(STATUS_OK, fat_seek_file(fd, 1, SEEK_CUR, &new_offset));
  TEST_ASSERT_EQUAL(new_offset, 2);
  TEST_ASSERT_EQUAL(STATUS_OK, fat_seek_file(fd, 0, SEEK_END, &new_offset));
  TEST_ASSERT_EQUAL(STATUS_ERR, fat_seek_file(fd, 1, SEEK_END, &new_offset));
  TEST_ASSERT_EQUAL(STATUS_ERR, fat_seek_file(0, 1, SEEK_SET, &new_offset));
  TEST_ASSERT_EQUAL(STATUS_ERR, fat_seek_file(fd, 1, 0xff, &new_offset));
  TEST_ASSERT_EQUAL(STATUS_OK, fat_free_cluster_table());
}
