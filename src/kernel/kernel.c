#include <stdint.h>
#include <stdio.h>

#include "kernel.h"
#include "sys_log.h"
#include "debug_interface.h"
#include "gpio.h"
#include "sys_timer.h"
#include "uart.h"
#include "example/example.h"
#include "bcm2835.h"
#include "emmc.h"

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

  SYS_LOG("starting predOS");
  SYS_LOG("New Message");
  emmc_init();
  /*
  emmc_block_t block[2];
  
  emmc_read_block(0, 2, block);
  SYS_LOG("block 0:");
  emmc_print_block(block[0]);
  SYS_LOG("block 1:");
  emmc_print_block(block[1]);

  block[0].buf[0] = 0x04030201;
  block[1].buf[0] = 0x0d0c0b0a;
  emmc_write_block(0, 2, block);
  emmc_read_block(0, 2, block);
  SYS_LOG("block 0:");
  emmc_print_block(block[0]);
  sys_timer_sleep(500000);
  SYS_LOG("block 1:");
  emmc_print_block(block[1]);

  */
  gpio_pulse(LED_PIN, 2); 
  while(1){
    //gpio_pulse(LED_PIN, 5);
    sys_timer_sleep(2000000);
  }

  return 0;
}

int kernel_init(void)
{
  int *bss = &__bss_start__;
  int* bss_end = &__bss_end__;

  while(bss < bss_end) *bss++ = 0;

  kernel_start();

  while(1);
}

