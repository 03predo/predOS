#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

#include "log_level.h"

#include "kernel.h"
#include "sys_log.h"
#include "debug_interface.h"
#include "gpio.h"
#include "sys_timer.h"
#include "uart.h"
#include "example/example.h"
#include "bcm2835.h"
#include "emmc.h"
#include "mmu.h"
#include "util.h"
#include "fat.h"

#define LED_PIN 16

extern int __bss_start__;
extern int __bss_end__;
extern void _enable_interrupts(void);

int kernel_init(void) __attribute__((naked)) __attribute__((section(".text.boot.kernel")));

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
  *(--APP_STACK) = (uint32_t)example_main; // context switch lr
  *(--APP_STACK) = 0x60000110; // SPSR

}

int kernel_start(){
  gpio_func(LED_PIN, GPIO_OUTPUT); 
  uart_init(3000000);
  _enable_interrupts();

  SYS_LOGV("starting predOS");
  fat_init();

  
  if(fat_create_file("log.txt") != STATUS_OK){
    SYS_LOGV("failed to create file");
    return -1;
  }
  
  uint8_t write_buf[] = "hello\n";
  if(fat_write_file("log.txt", write_buf, sizeof(write_buf)) == STATUS_OK){
    fat_directory_entry_t dir_entry;
    fat_get_dir_entry("log.txt", &dir_entry);
    fat_print_entry(dir_entry);

    uint8_t read_buf[7];
    fat_read_file("log.txt", read_buf, 7);
    SYS_LOGD("read_buf: %s", read_buf);
  }else{
    SYS_LOGV("file not found");
  }
  
  while(1){
    gpio_pulse(LED_PIN, 10);
    sys_timer_sleep(2000000);
  }

  return 0;
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

