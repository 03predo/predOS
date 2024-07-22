#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "log_level.h"

#include "kernel.h"
#include "sys_log.h"
#include "gpio.h"
#include "sys_timer.h"
#include "uart.h"
#include "bcm2835.h"
#include "emmc.h"
#include "mmu.h"
#include "util.h"
#include "fat.h"

#define LED_PIN 16
#define BUF_SIZE 2248

extern int __bss_start__;
extern int __bss_end__;
extern void _enable_interrupts(void);

static uint32_t* APP_STACK = (uint32_t*)0x1fe00000;

int kernel_init(void) __attribute__((naked)) __attribute__((section(".text.boot.kernel")));
extern void _software_interrupt_return(uint32_t stack_pointer);


/*
void setup_app_stack(){
  APP_STACK = (uint32_t*)0x8000;
  *(--APP_STACK) = 0xDEADBEEF; // app lr
  *(--APP_STACK) = 0xDEADBEEF; // r12
  *(--APP_STACK) = 0xDEADBEEF; // r11
  *(--APP_STACK) = 0xDEADBEEF; // r10
  *(--APP_STACK) = 0xDEADBEEF; // r9
  *(--APP_STACK) = 0xDEADBEEF; // r8
  *(--APP_STACK) = 0xDEADBEEF; // r7
  *(--APP_STACK) = 0xDEADBEEF; // r6
  *(--APP_STACK) = 0xDEADBEEF; // r5
  *(--APP_STACK) = 0xDEADBEEF; // r4
  *(--APP_STACK) = 0xDEADBEEF; // r3
  *(--APP_STACK) = 0xDEADBEEF; // r2
  *(--APP_STACK) = 0xDEADBEEF; // r1
  *(--APP_STACK) = 0xDEADBEEF; // r0
  //*(--APP_STACK) = (uint32_t)example_main; // context switch lr
  *(--APP_STACK) = 0x60000110; // SPSR

}
*/

int kernel_open(const char *pathname, int flags){
  int fd = -1;
  if(fat_open_file(pathname, flags, &fd) != STATUS_OK){
    SYS_LOGE("fat_open_file failed");
  }
  return fd;
}

int kernel_close(int file){
  if((file > 2) && (fat_close_file(file) != STATUS_OK)){
    SYS_LOGD("fat_close_file failed");
    return -1;
  }
  return 0;
}

int kernel_read(int file, char *ptr, int len){
  int bytes_read = -1;
  if(fat_read_file(file, ptr, len, &bytes_read) != STATUS_OK){
    SYS_LOGE("fat_open_file failed");
  }
  return bytes_read;
}

int kernel_write(int file, char *ptr, int len){
  if((file >= 0) && (file < 3)){
    uart_print(ptr, len);
    return len;
  }

  int bytes_written = -1;
  if(fat_write_file(file, ptr, len, &bytes_written) != STATUS_OK){
    SYS_LOGE("fat_open_file failed");
  }
  return bytes_written;
}

int kernel_lseek(int file, int offset, int whence){
  int new_offset = -1;
  if(fat_seek_file(file, offset, whence, &new_offset) != STATUS_OK){
    SYS_LOGE("fat_seek_file failed");
  }
  return new_offset;
}

int kernel_execv(const char *pathname, char *const argv[]){
  SYS_LOGI("filename: %s, arg[1]: %s", pathname, argv[1]);

  uint32_t arg_num = 0;
  while(argv[arg_num] != NULL){
    arg_num++;
  }
  SYS_LOGI("arg_num: %d", arg_num);
  char** app_argv = malloc(sizeof(char*)*(arg_num + 1));
  if(app_argv == NULL) return -1;
  for(uint32_t i = 0; i < arg_num; ++i){
    app_argv[i] = malloc(sizeof(char)*(strlen(argv[i]) + 1));
    if(app_argv == NULL) return -1;
    if(strcpy(app_argv[i], argv[i]) == NULL) return -1;
    SYS_LOGI("app_argv[%d]: %s", i, app_argv[i]);
  }
  app_argv[arg_num] = NULL; 
  
  uint32_t text_frame;
  if(mmu_allocate_frame(&text_frame) != STATUS_OK){
    SYS_LOGE("failed to allocate frame");
    return -1;
  }
  SYS_LOGI("got frame: %#x", text_frame);
  mmu_section_descriptor_t section = {
    .fields = {
      .descriptor_type = SECTION,
      .bufferable = 1,
      .cacheable = 1,
      .execute_never = 0,
      .domain = 0,
      .access_permission = 0b11,
      .type_extension = 0,
      .access_permission_extension = 0,
      .shareable = 0,
      .not_global = 0,
      .supersection = 0,
      .section_base = SECTION_BASE(text_frame),
    }
  };
  mmu_generic_descriptor_t desc;
  desc.raw = section.raw;
  mmu_system_page_table_set_entry(SECTION_BASE(text_frame), desc);
  
  int fd = kernel_open(pathname, O_RDWR);
  if(fd == -1){
    SYS_LOGE("open failed"); 
    return -1;
  }
  SYS_LOGI("opened file");

  int file_size = kernel_lseek(fd, 0, SEEK_END);
  if(file_size == -1){
    SYS_LOGE("lseek failed");
    return -1;
  }
  SYS_LOGI("file size: %d", file_size);

  int offset = kernel_lseek(fd, 0, SEEK_SET);
  if(offset == -1){
    SYS_LOGE("lseek failed");
    return -1;
  }

  int bytes_read = kernel_read(fd, (void*)text_frame, file_size);
  SYS_LOGI("bytes_read: %d", bytes_read);

  uint32_t stack_frame;
  if(mmu_allocate_frame(&stack_frame) != STATUS_OK){
    SYS_LOGE("failed to allocate frame");
    return -1;
  }
  SYS_LOGI("got frame: %#x", stack_frame);
  
  section = (mmu_section_descriptor_t) {
    .fields = {
      .descriptor_type = SECTION,
      .bufferable = 1,
      .cacheable = 1,
      .execute_never = 0,
      .domain = 0,
      .access_permission = 0b11,
      .type_extension = 0,
      .access_permission_extension = 0,
      .shareable = 0,
      .not_global = 0,
      .supersection = 0,
      .section_base = SECTION_BASE(stack_frame),
    }
  };
  desc.raw = section.raw;
  mmu_system_page_table_set_entry(SECTION_BASE(stack_frame), desc);
  desc.raw = 0;
  mmu_system_page_table_set_entry(SECTION_BASE(text_frame), desc);

  mmu_small_page_descriptor_t small_page = {
    .fields = {
      .descriptor_type = SMALL_PAGE_EXECUTABLE,
      .bufferable = 1,
      .cacheable = 1,
      .access_permission = 0b11,
      .access_permission_extension = 0,
      .shareable  = 0,
      .not_global = 0,
      .small_page_base = SMALL_PAGE_BASE(text_frame),
    }
  };

  for(int i = 0; i < (256 - SMALL_PAGE_BASE(0x8000)); ++i){
    small_page.fields.small_page_base = SMALL_PAGE_BASE(text_frame) + i;
    mmu_root_coarse_page_table_set_entry(SMALL_PAGE_BASE(0x8000) + i, small_page);
  }
  uint32_t* instruction = (uint32_t*)0x8128;
  SYS_LOGI("instruction: %#x", *instruction);
  SYS_LOGI("app_argv: %#x", app_argv);
  
  uint32_t* app_stack = (uint32_t*)(stack_frame + 0x100000);
  *(--app_stack) = (uint32_t)_exit; // lr
  *(--app_stack) = 0xDEADBEEF; // r12
  *(--app_stack) = 0xDEADBEEF; // r11
  *(--app_stack) = 0xDEADBEEF; // r10
  *(--app_stack) = 0xDEADBEEF; // r9
  *(--app_stack) = 0xDEADBEEF; // r8
  *(--app_stack) = 0xDEADBEEF; // r7
  *(--app_stack) = 0xDEADBEEF; // r6
  *(--app_stack) = 0xDEADBEEF; // r5
  *(--app_stack) = 0xDEADBEEF; // r4
  *(--app_stack) = 0xDEADBEEF; // r3
  *(--app_stack) = 0xDEADBEEF; // r2
  *(--app_stack) = 0xDEADBEEF; // r1
  *(--app_stack) = (uint32_t)app_argv; // r0
  *(--app_stack) = (uint32_t)0x8000; // context switch lr
  *(--app_stack) = 0x60000110; // SPSR
  _software_interrupt_return((uint32_t)app_stack);
  while(1);
}

int kernel_start(){
  gpio_func(LED_PIN, GPIO_OUTPUT); 
  uart_init(115200);
  _enable_interrupts();
  fat_init();
  mmu_frame_table_init();
  sys_timer_sleep(1000);
  SYS_LOGI("starting predOS (v%s)", VERSION); 

  char *args[]={"example", "hello", "world", NULL};
  SYS_LOGI("args: %#x", args);
  execv(args[0], args);
  while(1);
}

int kernel_init(void)
{
  int *bss = &__bss_start__;
  int* bss_end = &__bss_end__;

  while(bss < bss_end) *bss++ = 0;

  mmu_init();

  kernel_start();

  while(1);
}

