#include "bcm2835.h"
#include "gpio.h"
#include "arm_timer.h"
#include "uart.h"

#define LED_PIN 16

void __attribute__((interrupt("UNDEF"))) undefined_instruction_handler(void){
    while(1);
}

void __attribute__((interrupt("SWI"))) software_interrupt_handler(void){
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
