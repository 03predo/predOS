#include <stdint.h>
#include <stdio.h>
#include "kernel.h"
#include "sys_log.h"
#include "gpio.h"
#include "sys_timer.h"
#include "arm_timer.h"
#include "uart.h"
#include "example/example.h"

#define LED_PIN 16

#define GET_LR(lr) asm inline ("mov %0, lr" : "=r" (lr) : : )
#define GET_SP(sp) asm inline ("mov %0, sp" : "=r" (sp) : : )


extern void _enable_interrupts(void);

static inline uint32_t get_link_register(void){
  int lr = 0;
  asm inline ("mov %0, lr" : "=r" (lr) : : );
  return lr;

}

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


int kernel_main(){
  gpio_func(LED_PIN, GPIO_OUTPUT); 
  arm_timer_init(0x400);
  _enable_interrupts();

  uart_init(115200);
  uint32_t sp;
  GET_SP(sp);
  setup_app_stack();
  SYS_LOG("hello from predOS, sp=%#x", APP_STACK);

  asm inline("SWI 1");
  
  uint32_t counter = 0;
  while(1){
    SYS_LOG("counter = %d", counter);
    
    sys_timer_sleep(1000000);  

    counter++;
  }

  return 0;
}
