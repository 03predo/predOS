#include "bcm2835.h"
#include "gpio.h"
#include "arm_timer.h"
#include "uart.h"
#include "sys_log.h"
#include "util.h"
#include "kernel.h"

#define LED_PIN 16

extern void _context_switch(uint32_t prev_stack_pointer);

void __attribute__((interrupt("UNDEF"))) undefined_instruction_handler(void){
    SYS_LOG("UNDEFINED INSTRUCTION");
    while(1);
}

void software_interrupt_handler(uint32_t prev_stack_pointer){
  SYS_LOG("SOFTWARE INTERRUPT");
  SYS_LOG("prev_stack_pointer: %#x, link_reg: %#x, prev_mode: %#x", prev_stack_pointer, *((uint32_t*)(prev_stack_pointer + 1)), *((uint32_t*)prev_stack_pointer));
  SYS_LOG("lr: %#x, spsr: %#x, sp: %#x", *(APP_STACK + 1), *(APP_STACK), APP_STACK);
  _context_switch((uint32_t)APP_STACK);
  
  while(1);
}

void __attribute__((interrupt("ABORT"))) prefetch_abort_handler(void){
  while(1);
}

void __attribute__((interrupt("ABORT"))) data_abort_handler(void){
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
  while(1);
}
