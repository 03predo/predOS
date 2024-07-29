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
#include "proc.h"

#define LED_PIN 16
#define MAX_PROCESSES 10

extern int __bss_start__;
extern int __bss_end__;

extern void _kernel_context_switch(uint32_t stack_pointer);

int kernel_init(void) __attribute__((naked)) __attribute__((section(".text.boot.kernel")));

static process_control_block_t pcb_list[MAX_PROCESSES];
static process_control_block_t* pcb_curr = NULL;

status_t kernel_schedule(uint32_t* pid){
  for(uint32_t i = 0; i < MAX_PROCESSES; ++i){
    *pid = (pcb_curr->id + (i + 1)) % MAX_PROCESSES;
    if(pcb_list[*pid].state == READY){
      SYS_LOGI("found ready process %d", *pid);
      return STATUS_OK;
    }else if(pcb_list[*pid].state == BLOCKED){
      if(pcb_list[*pid].timestamp < sys_uptime()){
        pcb_list[*pid].state = READY;
        SYS_LOGI("unblocking process %d", *pid);
        return STATUS_OK;
      }
    }
  }
  *pid = MAX_PROCESSES;
  SYS_LOGD("unable to find ready process");
  return STATUS_ERR;
}

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
  SYS_LOGI("filename: %s", pathname);

  process_control_block_t* pcb = NULL;
  if(pcb_curr == NULL){ // case should only occur on startup
    for(uint32_t i = 0; i < MAX_PROCESSES; ++i){
      if(pcb_list[i].state == UNUSED){
        pcb = &pcb_list[i];
        break;
      }
    }
    if(pcb == NULL){
      SYS_LOGE("failed to find free pcb");
      return -1;
    }
    proc_create(pcb);
  }else{
    pcb = pcb_curr;
    free(pcb->argv);
  }

  uint32_t arg_num = 0;
  while(argv[arg_num] != NULL){
    arg_num++;
  }
  SYS_LOGI("arg_num: %d", arg_num);
  pcb->argv = malloc(sizeof(char*)*(arg_num + 1));
  if(pcb->argv == NULL) return -1;
  for(uint32_t i = 0; i < arg_num; ++i){
    pcb->argv[i] = malloc(sizeof(char)*(strlen(argv[i]) + 1));
    if(pcb->argv == NULL) return -1;
    if(strcpy(pcb->argv[i], argv[i]) == NULL) return -1;
    SYS_LOGI("pcb->argv[%d]: %s", i, pcb->argv[i]);
  }
  pcb->argv[arg_num] = NULL; 
  
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
      .section_base = SECTION_BASE(pcb->text_frame),
    }
  };
  mmu_generic_descriptor_t desc;
  desc.raw = section.raw;
  mmu_system_page_table_set_entry(SECTION_BASE(pcb->text_frame), desc);
  
  int fd = kernel_open(pathname, O_RDWR);
  if(fd == -1){
    SYS_LOGE("open failed"); 
    proc_destroy(pcb);
    return -1;
  }
  SYS_LOGI("opened file");

  int file_size = kernel_lseek(fd, 0, SEEK_END);
  if(file_size == -1){
    SYS_LOGE("lseek failed");
    proc_destroy(pcb);
    return -1;
  }
  SYS_LOGI("file size: %d", file_size);

  int offset = kernel_lseek(fd, 0, SEEK_SET);
  if(offset == -1){
    SYS_LOGE("lseek failed");
    proc_destroy(pcb);
    return -1;
  }

  int bytes_read = kernel_read(fd, (void*)pcb->text_frame, file_size);
  SYS_LOGI("bytes_read: %d", bytes_read);
 
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
      .section_base = SECTION_BASE(pcb->stack_frame),
    }
  };
  desc.raw = section.raw;
  mmu_system_page_table_set_entry(SECTION_BASE(pcb->stack_frame), desc);
  desc.raw = 0;
  mmu_system_page_table_set_entry(SECTION_BASE(pcb->text_frame), desc);

  mmu_small_page_descriptor_t small_page = {
    .fields = {
      .descriptor_type = SMALL_PAGE_EXECUTABLE,
      .bufferable = 1,
      .cacheable = 1,
      .access_permission = 0b11,
      .access_permission_extension = 0,
      .shareable  = 0,
      .not_global = 0,
      .small_page_base = SMALL_PAGE_BASE(pcb->text_frame),
    }
  };

  for(int i = 0; i < (256 - SMALL_PAGE_BASE(0x8000)); ++i){
    small_page.fields.small_page_base = SMALL_PAGE_BASE(pcb->text_frame) + i;
    mmu_root_coarse_page_table_set_entry(SMALL_PAGE_BASE(0x8000) + i, small_page);
  }
  uint32_t* instruction = (uint32_t*)0x8128;
  SYS_LOGI("instruction: %#x", *instruction);
  
  uint32_t* app_stack = pcb->stack_pointer;
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
  *(--app_stack) = (uint32_t)pcb->argv; // r0
  *(--app_stack) = (uint32_t)0x8000; // context switch lr
  *(--app_stack) = 0x60000110; // SPSR
  pcb_curr->state = READY;
  pcb->state = RUNNING;
  pcb_curr = pcb;
  _kernel_context_switch((uint32_t)app_stack);
  return -1;
}

int kernel_fork(uint32_t sp, uint32_t fp){
  process_control_block_t* pcb = NULL;
  for(uint32_t i = 0; i < MAX_PROCESSES; ++i){
    if(pcb_list[i].state == UNUSED){
      pcb = &pcb_list[i];
      break;
    }
  }
  if(pcb == NULL){
    SYS_LOGE("failed to find free pcb");
    return -1;
  }
  proc_create(pcb);

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
      .section_base = SECTION_BASE(pcb->stack_frame),
    }
  };
  mmu_generic_descriptor_t desc;
  desc.raw = section.raw;
  mmu_system_page_table_set_entry(SECTION_BASE(pcb->stack_frame), desc);
 
  section.fields.section_base = SECTION_BASE(pcb->text_frame); 
  desc.raw = section.raw;
  mmu_system_page_table_set_entry(SECTION_BASE(pcb->text_frame), desc);

  section.fields.section_base = SECTION_BASE(pcb_curr->text_frame); 
  desc.raw = section.raw;
  mmu_system_page_table_set_entry(SECTION_BASE(pcb_curr->text_frame), desc);

  memcpy((void*)pcb->text_frame, (void*)pcb_curr->text_frame, SECTION_SIZE); 
  memcpy((void*)pcb->stack_frame, (void*)pcb_curr->stack_frame, SECTION_SIZE);  

  desc.raw = 0;
  mmu_system_page_table_set_entry(SECTION_BASE(pcb->text_frame), desc);
  mmu_system_page_table_set_entry(SECTION_BASE(pcb_curr->text_frame), desc);

  pcb->stack_pointer = (uint32_t*)((sp - pcb_curr->stack_frame) + pcb->stack_frame);
  pcb->stack_pointer[13] = (uint32_t)((fp - pcb_curr->stack_frame) + pcb->stack_frame); // set fp
  pcb->stack_pointer[2] = 0; // set r0
  pcb->state = READY; 
  return pcb->id;
}

void kernel_yield(uint32_t* sp){
  pcb_curr->stack_pointer = sp;
  pcb_curr->state = READY;

  uint32_t pid = MAX_PROCESSES;
  while(kernel_schedule(&pid) != STATUS_OK);
  pcb_curr = &pcb_list[pid]; 
  _kernel_context_switch((uint32_t)pcb_curr->stack_pointer); 
}

void kernel_usleep(uint32_t* sp, uint32_t timeout){
  pcb_curr->stack_pointer = sp;
  pcb_curr->state = BLOCKED;
  pcb_curr->timestamp = sys_uptime() + timeout;

  uint32_t pid = MAX_PROCESSES;
  while(kernel_schedule(&pid) != STATUS_OK);
  pcb_curr = &pcb_list[pid];
  _kernel_context_switch((uint32_t)pcb_curr->stack_pointer); 
}


int kernel_start(){
  if(gpio_func(LED_PIN, GPIO_OUTPUT) != STATUS_OK) exit(-1);
  if(uart_init(115200) != STATUS_OK) exit(-1);
  if(fat_init() != STATUS_OK) exit(-1); 
  if(mmu_frame_table_init() != STATUS_OK) exit(-1);

  sys_timer_sleep(1000);
  SYS_LOGI("starting predOS (v%s)", VERSION); 

  for(uint32_t i = 0; i < MAX_PROCESSES; ++i){
    pcb_list[i] = (process_control_block_t) {
      .id = i,
      .state = UNUSED,
      .argv = NULL,
      .text_frame = 0,
      .stack_frame = 0,
      .stack_pointer = NULL,
      .timestamp = 0
    };
  }

  char *args[] = {"init", NULL};
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


