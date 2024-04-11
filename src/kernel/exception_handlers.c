#include "bcm2835.h"
#include "gpio.h"
#include "arm_timer.h"
#include "uart.h"
#include "sys_log.h"
#include "util.h"
#include "kernel.h"

#define LED_PIN 16

extern void _kernel_context_switch(uint32_t prev_stack_pointer);

void __attribute__((interrupt("UNDEF"))) undefined_instruction_handler(uint32_t spsr, uint32_t lr){
  SYS_LOG("UNDEFINED INSTRUCTION");
  SYS_LOG("spsr: %#x, lr: %#x\n", spsr, lr); 
  while(1);
}

void software_interrupt_handler(uint32_t sp){
  SYS_LOG("SOFTWARE INTERRUPT");
  SYS_LOG("prev: sp=%#x, lr=%#x, spsr=%#x", sp, *((uint32_t*)(sp + 4)), *((uint32_t*)sp));
  SYS_LOG("new: sp=%#x, lr=%#x, spsr=%#x", APP_STACK, *((uint32_t*)(APP_STACK + 1)), *(APP_STACK));
  _kernel_context_switch((uint32_t)APP_STACK); 
  while(1);
}

void __attribute__((interrupt("ABORT"))) prefetch_abort_handler(void){
  SYS_LOG("PREFETCH ABORT");
  while(1);
}

void __attribute__((interrupt("ABORT"))) data_abort_handler(void){
  SYS_LOG("DATA ABORT");
  while(1);
}

void __attribute__((interrupt("IRQ"))) interrupt_handler(void){

  static int lit = 0;
  if(ARM_TIMER->MASKED_IRQ){
    ARM_TIMER->IRQ_CLEAR = 1;
    if(lit){
      gpio_clear(LED_PIN);
      lit = 0;
    }else{
      gpio_set(LED_PIN);
      lit = 1;
    }
  }
}

void __attribute__((interrupt("FIQ"))) fast_interrupt_handler(void){
  SYS_LOG("FAST INTERRUPT");
  while(1);
}
