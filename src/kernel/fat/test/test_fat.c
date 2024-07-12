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

uint32_t* disk;

void setUp (void) {}
void tearDown (void) {}

void print_block(){
  for(int j = 0; j < 32; ++j){
    printf("0x%04x   ", j*16);
    for(int k = 0; k < 4; ++k){
      printf("%02x %02x %02x %02x ",
          BYTE0(disk[j*4 + k]),
          BYTE1(disk[j*4 + k]),
          BYTE2(disk[j*4 + k]),
          BYTE3(disk[j*4 + k])
      );
      if(k == 1) printf(" ");
    }
    printf("\n");
  }
}


status_t mock_emmc_init(int cmock_num_calls){
  printf("cmock_num_calls: %d\n", cmock_num_calls);
  return STATUS_OK;
}


status_t mock_emmc_read_block(uint32_t start_block_address, uint16_t num_blocks, emmc_block_t* block, int cmock_num_calls){
  uint32_t* start_block = &disk[start_block_address * (EMMC_BLOCK_SIZE / sizeof(uint32_t))];
  for(uint32_t i = 0; i < num_blocks * (EMMC_BLOCK_SIZE / sizeof(uint32_t)); ++i){
      block[i / (EMMC_BLOCK_SIZE / sizeof(uint32_t))].buf[i % EMMC_BLOCK_SIZE] = start_block[i];
  }
  return STATUS_OK;
}


void test(void){
  sys_uptime_IgnoreAndReturn(0);
  sys_timer_sleep_Ignore();

  const char img_file[] = IMG_DIR"/predOS.img";
  printf("img_file: %s\n", img_file);

  struct stat st;
  stat(img_file, &st);
  printf("file size: %ld\n", st.st_size);

  disk = calloc(st.st_size / sizeof(uint32_t), sizeof(uint32_t));
  
  if(disk == NULL){
    printf("failed to allocate buf");
    return;
  }
  int fd = open(IMG_DIR"/predOS.img", O_RDWR);
  if(fd == -1){
    printf("failed to open file (%d)\n", errno);
    return;
  }

  int bytes_read = read(fd, disk, st.st_size);
  printf("bytes_read: %d\n", bytes_read);
  print_block();
  emmc_init_Stub(mock_emmc_init);
  emmc_read_block_Stub(mock_emmc_read_block); 
  TEST_ASSERT_EQUAL(fat_init(), STATUS_OK);
}

