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

int kernel_init(void) __attribute__((naked)) __attribute__((section(".text.boot.kernel")));

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

int kernel_start(){
  gpio_func(LED_PIN, GPIO_OUTPUT); 
  uart_init(115200);
  _enable_interrupts();

  fat_init();

  sys_timer_sleep(1000);
  SYS_LOGV("starting predOS (v%s)", VERSION);
  printf("hello world\n");
  int fd = open("config.txt", O_RDWR); 
  if(fd == -1){
    SYS_LOGE("open failed");
    exit(-1);
  }
  SYS_LOGV("open done, fd=%d", fd);

  char buf[9];
  int bytes_read = read(fd, buf, 8);
  if(bytes_read == -1){
    SYS_LOGE("read failed");
    exit(-1);
  }

  buf[8] = '\0';
  SYS_LOGV("read: %s", buf);

  if(close(fd) != 0){
    SYS_LOGE("close failed");
    exit(-1);
  }
  SYS_LOGV("close done");

  int log_fd = open("log.txt", O_RDWR | O_CREAT);
  if(log_fd == -1){
    SYS_LOGE("open failed");
    exit(-1);
  }
  SYS_LOGV("open done, fd=%d", fd);

  char log_buf[] = "log things";
  int bytes_written = write(log_fd, log_buf, sizeof(log_buf));
  if(bytes_written == -1){
    SYS_LOGE("write failed");
    exit(-1);
  }

  if(close(log_fd) != 0){
    SYS_LOGE("close failed");
    exit(-1);
  }
  SYS_LOGV("close done");

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

