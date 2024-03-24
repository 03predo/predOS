#include <stdint.h>
#include <stdio.h>
#include "kernel.h"
#include "sys_log.h"
#include "util.h"
#include "gpio.h"
#include "sys_timer.h"
#include "arm_timer.h"
#include "uart.h"
#include "example/example.h"

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
  arm_timer_init(0x400);
  _enable_interrupts();

  uart_init(115200);

  SYS_LOG("starting predOS");

  setup_app_stack();

  asm inline("SVC 1");

  SYS_LOG("returned from SWI"); 

  while(1);

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

