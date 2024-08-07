#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include "kernel.h"
#include "sys_log.h"
#include "gpio.h"
#include "sys_timer.h"
#include "arm_timer.h"
#include "uart.h"
#include "util.h"
#include "fat.h"
#include "proc.h"
#include "elf.h"

#define LED_PIN 16
#define MAX_PROCESSES 10

extern int __bss_start__;
extern int __bss_end__;

int kernel_init(void) __attribute__((naked)) __attribute__((section(".text.boot.kernel")));

static process_control_block_t pcb_list[MAX_PROCESSES];
process_control_block_t* pcb_curr = NULL;

typedef struct {
  pid_t pid;
  int fd;
  uint32_t size;
  uint8_t* buf;
} read_transaction_t;

read_transaction_t read_queue[MAX_PROCESSES];

pid_t wait_queue[MAX_PROCESSES];

status_t kernel_schedule(pid_t* pid){
  for(uint32_t i = 0; i < MAX_PROCESSES; ++i){
    *pid = (pcb_curr->pid + (i + 1)) % MAX_PROCESSES;
    if(pcb_list[*pid].state == READY){
      SYS_LOGD("found ready process %d", *pid);
      return STATUS_OK;
    }else if(pcb_list[*pid].state == SLEEP){
      if(pcb_list[*pid].timestamp < sys_uptime()){
        pcb_list[*pid].state = READY;
        SYS_LOGD("unblocking process %d", *pid);
        return STATUS_OK;
      }
    }
  }
  *pid = MAX_PROCESSES;
  return STATUS_ERR;
}

status_t kernel_context_save(uint32_t* sp){
  if(pcb_curr == NULL) return STATUS_ERR;
  if(pcb_curr->state != UNUSED) pcb_curr->stack_pointer = sp;
  return STATUS_OK;
}

status_t kernel_context_switch(uint32_t* sp){
  STATUS_OK_OR_RETURN(proc_frame_map(pcb_curr));
  *sp = (uint32_t)pcb_curr->stack_pointer;
  pcb_curr->state = RUNNING;
  return STATUS_OK;
}

status_t kernel_read_queue_update(int fd, uint32_t size){
  for(uint32_t i = 0; i < MAX_PROCESSES; ++i){
    if((read_queue[i].pid != -1) && (read_queue[i].fd == fd) && (read_queue[i].size <= size)){
      if(fd != STDIN_FILENO){ 
        return STATUS_OK;
      }
      
      process_control_block_t* pcb = &pcb_list[read_queue[i].pid];
      STATUS_OK_OR_RETURN(proc_frame_map(pcb));

      int bytes_read = -1;
      STATUS_OK_OR_RETURN(uart_read(read_queue[i].buf, read_queue[i].size, &bytes_read));
      if(bytes_read != read_queue[i].size){
        SYS_LOGE("uart_read failed");
        return STATUS_ERR;
      }
      
      pcb->state = READY; 
      pcb->stack_pointer[2] = bytes_read;
      STATUS_OK_OR_RETURN(proc_frame_map(pcb_curr));
      return STATUS_OK;
    }
  }
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
  if(file == STDIN_FILENO){
    if(uart_read(ptr, len, &bytes_read) != STATUS_OK){
      SYS_LOGE("uart_read failed");
      return -1;
    }
    if(bytes_read == len){
      return bytes_read;
    }
    for(uint32_t i = 0; i < MAX_PROCESSES; ++i){
      if(read_queue[i].pid == -1){
        read_queue[i].pid = pcb_curr->pid;
        read_queue[i].fd = STDIN_FILENO;
        read_queue[i].buf = ptr;
        read_queue[i].size = len;
        break;
      }
    }
    pcb_curr->state = BLOCKED;
    pid_t pid = MAX_PROCESSES;
    while(kernel_schedule(&pid) != STATUS_OK);
    pcb_curr = &pcb_list[pid];

    return -1;
  }
  if(fat_read_file(file, ptr, len, &bytes_read) != STATUS_OK){
    SYS_LOGE("fat_read_file failed");
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

status_t kernel_set_args(process_control_block_t* pcb, char* const argv[]){
  uint32_t arg_num = 0;
  while(argv[arg_num] != NULL){
    SYS_LOGD("argv[%d]=%s", arg_num, argv[arg_num]);
    arg_num++;
  }

  pcb->stack_pointer -= arg_num + 1;
  pcb->argv = (char**)pcb->stack_pointer;
  SYS_LOGD("pcb->argv: %#x", pcb->argv);

  for(uint32_t i = 0; i < arg_num; ++i){
    uint32_t arg_size = 0;
    while(argv[i][arg_size] != '\0'){
      arg_size++;
    }
    arg_size++;
    SYS_LOGD("arg_size[%d]: %d", i, arg_size);

    uint32_t offset = (arg_size + sizeof(uint32_t) - 1) / sizeof(uint32_t);

    pcb->stack_pointer -= offset + 1;
    pcb->argv[i] = (char*)pcb->stack_pointer;
    
    for(uint32_t j = 0; j < arg_size; ++j){
      pcb->argv[i][j] = argv[i][j];
    }
    pcb->argv[i][arg_size] = '\0';
  }
  pcb->argv[arg_num] = NULL;

  arg_num = 0;
  while(pcb->argv[arg_num] != NULL){
    SYS_LOGD("pcb->argv[%d]: %s", arg_num, pcb->argv[arg_num]);
    arg_num++;
  }
  return STATUS_OK;
}

int kernel_execv(const char *pathname, char *const argv[]){
  SYS_LOGD("filename: %s", pathname); 

  int fd = kernel_open(pathname, O_RDWR);
  if(fd == -1){
    SYS_LOGE("open failed"); 
    //proc_destroy(pcb);
    return -1;
  }
  SYS_LOGD("opened file");

  int file_size = kernel_lseek(fd, 0, SEEK_END);
  if(file_size == -1){
    SYS_LOGE("lseek failed");
    return -1;
  }
  SYS_LOGD("file size: %d", file_size);

  int offset = kernel_lseek(fd, 0, SEEK_SET);
  if(offset == -1){
    SYS_LOGE("lseek failed");
    return -1;
  }

  process_control_block_t* pcb = pcb_curr;
  pcb_curr->virtual_stack_frame = pcb_curr->stack_frame;
  pcb_curr->stack_pointer = (uint32_t*)(pcb_curr->stack_frame + SECTION_SIZE);
  STATUS_OK_OR_RETURN(proc_frame_write_enable(pcb_curr));

  kernel_set_args(pcb, argv);
  int bytes_read = kernel_read(fd, (void*)pcb->text_frame, file_size);
  SYS_LOGD("bytes_read: %d", bytes_read); 
   
  *(--pcb->stack_pointer) = (uint32_t)_exit; // lr
  *(--pcb->stack_pointer) = 0xDEADBEEF; // r12
  *(--pcb->stack_pointer) = 0xDEADBEEF; // r11
  *(--pcb->stack_pointer) = 0xDEADBEEF; // r10
  *(--pcb->stack_pointer) = 0xDEADBEEF; // r9
  *(--pcb->stack_pointer) = 0xDEADBEEF; // r8
  *(--pcb->stack_pointer) = 0xDEADBEEF; // r7
  *(--pcb->stack_pointer) = 0xDEADBEEF; // r6
  *(--pcb->stack_pointer) = 0xDEADBEEF; // r5
  *(--pcb->stack_pointer) = 0xDEADBEEF; // r4
  *(--pcb->stack_pointer) = 0xDEADBEEF; // r3
  *(--pcb->stack_pointer) = 0xDEADBEEF; // r2
  *(--pcb->stack_pointer) = 0xDEADBEEF; // r1
  *(--pcb->stack_pointer) = (uint32_t)pcb->argv; // r0
  *(--pcb->stack_pointer) = (uint32_t)0x8000; // context switch lr
  *(--pcb->stack_pointer) = 0x60000110; // SPSR

  pcb_curr->state = READY;
  pcb_curr = pcb;

  STATUS_OK_OR_RETURN(proc_frame_write_disable(pcb_curr));
  return -1;
}

void kernel_exit(int exit_status){ 
  SYS_LOGD("process %d exited with %d", pcb_curr->pid, exit_status);
  if(proc_frame_write_disable(pcb_curr) != STATUS_OK){
    SYS_LOGE("failed to frame write disable");
    return;
  }
  if(proc_destroy(pcb_curr) != STATUS_OK){
    SYS_LOGE("failed to destroy pcb");
    return;
  }
  pcb_curr->state = UNUSED;

  for(uint32_t i = 0; i < MAX_PROCESSES; ++i){
    if(wait_queue[i] == pcb_curr->parent_pid){
      SYS_LOGD("unblocking parent %d", pcb_curr->parent_pid);
      process_control_block_t* pcb_parent = &pcb_list[wait_queue[i]];
      if(proc_frame_map(pcb_parent)){
        SYS_LOGE("failed to frame write enable");
        return;
      }
      uint32_t* child_exit_status = (uint32_t*)pcb_parent->stack_pointer[2];
      SYS_LOGD("child_exit_status: %#x", child_exit_status);
      pcb_parent->stack_pointer[2] = pcb_curr->pid;
      pcb_parent->stack_pointer[3] = exit_status;
      pcb_parent->state = READY;
      wait_queue[i] = -1;
    }
  }

  pid_t pid = MAX_PROCESSES;
  while(kernel_schedule(&pid) != STATUS_OK);
  pcb_curr = &pcb_list[pid];
}

int kernel_fork(){
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
  pcb->virtual_stack_frame = pcb_curr->stack_frame;
  STATUS_OK_OR_RETURN(proc_frame_write_enable(pcb));
  STATUS_OK_OR_RETURN(proc_frame_write_enable(pcb_curr));

  memcpy((void*)pcb->text_frame, (void*)pcb_curr->text_frame, SECTION_SIZE); 
  memcpy((void*)pcb->stack_frame, (void*)pcb_curr->stack_frame, SECTION_SIZE);  

  pcb->stack_pointer = (uint32_t*)((((uint32_t)pcb_curr->stack_pointer) - pcb_curr->stack_frame) + pcb->stack_frame);
  pcb->stack_pointer[2] = 0; // set r0
  pcb->stack_pointer = (uint32_t*)(pcb_curr->stack_pointer);
  pcb->parent_pid = pcb_curr->pid;
  pcb->state = READY; 

  STATUS_OK_OR_RETURN(proc_frame_write_disable(pcb));
  return pcb->pid;
}

int kernel_wait(){
  pcb_curr->state = BLOCKED;

  int index = -1;
  for(uint32_t i = 0; i < MAX_PROCESSES; ++i){
    if(wait_queue[i] == -1){
      index = i;
      break;
    }
  }

  if(index == -1){
    SYS_LOGE("can't place pid in wait queue");
    return -1;
  }

  wait_queue[index] = pcb_curr->pid;

  pid_t pid = MAX_PROCESSES;
  while(kernel_schedule(&pid) != STATUS_OK);
  pcb_curr = &pcb_list[pid];
  return -1;
}

int kernel_yield(){
  pcb_curr->state = READY;

  pid_t pid = MAX_PROCESSES;
  while(kernel_schedule(&pid) != STATUS_OK);
  pcb_curr = &pcb_list[pid];
  return 0;
}

int kernel_usleep(uint32_t timeout){
  pcb_curr->state = SLEEP;
  pcb_curr->timestamp = sys_uptime() + timeout;

  pid_t pid = MAX_PROCESSES;
  while(kernel_schedule(&pid) != STATUS_OK);
  pcb_curr = &pcb_list[pid];
  return 0;
}

int kernel_start(){
  if(gpio_func(LED_PIN, GPIO_OUTPUT) != STATUS_OK) exit(-1);
  //if(arm_timer_init(0x800) != STATUS_OK) exit(-1);
  if(uart_init(115200) != STATUS_OK) exit(-1);
  if(fat_init() != STATUS_OK) exit(-1); 
  if(mmu_frame_table_init() != STATUS_OK) exit(-1);

  sys_timer_sleep(1000);
  SYS_LOGI("starting predOS (v%s)", VERSION); 

  for(uint32_t i = 0; i < MAX_PROCESSES; ++i){
    pcb_list[i] = (process_control_block_t) {
      .pid = i,
      .parent_pid = -1,
      .state = UNUSED,
      .argv = NULL,
      .text_frame = 0,
      .stack_frame = 0,
      .stack_pointer = NULL,
      .timestamp = 0
    };
    wait_queue[i] = -1;
    read_queue[i].pid = -1;
  }

  pcb_curr = &pcb_list[0];
  pcb_curr->parent_pid = -1;
  proc_create(pcb_curr);

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


