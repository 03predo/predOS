#include "bcm2835.h"
#include "gpio.h"
#include "arm_timer.h"
#include "uart.h"
#include "sys_log.h"
#include "util.h"
#include "svc.h"

#define LED_PIN 16

extern void _kernel_context_switch(uint32_t stack_pointer);
extern void _mmu_disable();

void __attribute__((interrupt("UNDEF"))) undefined_instruction_handler(uint32_t spsr, uint32_t lr){
  SYS_LOG("UNDEFINED INSTRUCTION");
  SYS_LOG("spsr: %#x, lr: %#x\n", spsr, lr); 
  while(1){
    gpio_pulse(LED_PIN, 1);
    sys_timer_sleep(1000000);
  }
  while(1);
}

void software_interrupt_handler(uint32_t sp){
  asm inline("cpsie i"); 
  SYS_LOGD("SOFTWARE INTERRUPT");

  uint32_t spsr = *((uint32_t*)sp);
  uint32_t lr = *((uint32_t*)(sp + 4));
  uint32_t svc = (*((uint32_t*)(lr - 4))) & 0xffffff;
  SYS_LOGD("prev: sp=%#x, lr=%#x, spsr=%#x, svc=%#x", sp, lr, spsr, svc);

  if(svc_handler((uint32_t*)(sp + 8), svc) != STATUS_OK){
    SYS_LOGE("svc_handler failed");
    while(1){
      gpio_pulse(LED_PIN, 1);
      sys_timer_sleep(1000000);
    }
  }
  
  _kernel_context_switch(sp);
}

void __attribute__((interrupt("ABORT"))) prefetch_abort_handler(uint32_t lr){
  asm inline("cpsie i");
  SYS_LOG("PREFETCH ABORT: %#x", lr);

  while(1){
    gpio_pulse(LED_PIN, 3);
    sys_timer_sleep(1000000);
  }
}

void data_abort_handler(uint32_t lr, uint32_t dfsr){
  asm inline("cpsie i");
  SYS_LOG("DATA ABORT: %#x, %#x", lr, dfsr);
  while(1){
    gpio_pulse(LED_PIN, 4);
    sys_timer_sleep(1000000);
  }
}

void __attribute__((interrupt("IRQ"))) interrupt_handler(void){
  if(!(AUX->MINI_UART_IRQ_STATUS & 0b001)){
    uart_irq_handler();
  }else{
    gpio_pulse(LED_PIN, 2);
  }

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
