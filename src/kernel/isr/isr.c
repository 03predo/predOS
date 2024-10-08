#include <unistd.h>
#include "bcm2835.h"
#include "kernel.h"
#include "gpio.h"
#include "arm_timer.h"
#include "uart.h"
#include "sys_log.h"
#include "util.h"
#include "svc.h"

#define LED_PIN 16

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

uint32_t software_interrupt_handler(uint32_t prev_sp){
  asm inline("cpsie i"); 
  uint32_t spsr = *((uint32_t*)prev_sp);
  uint32_t lr = *((uint32_t*)(prev_sp + 4));
  uint32_t svc = (*((uint32_t*)(lr - 4))) & 0xffffff;
  SYS_LOGD("prev: prev_sp=%#x, lr=%#x, spsr=%#x, svc=%#x", prev_sp, lr, spsr, svc);

  if(kernel_context_save((uint32_t*)prev_sp) != STATUS_OK) return prev_sp;

  if(svc_handler((uint32_t*)(prev_sp + 8), svc) != STATUS_OK){
    SYS_LOGE("svc_handler failed");
    while(1){
      gpio_pulse(LED_PIN, 1);
      sys_timer_sleep(1000000);
    }
  }
  
  uint32_t sp = 0;
  if(kernel_context_switch(&sp) != STATUS_OK){
    SYS_LOGE("context switch failed");
    return prev_sp;
  }
  return sp;
}

void __attribute__((interrupt("ABORT"))) prefetch_abort_handler(uint32_t lr){
  asm inline("cpsie i");
  SYS_LOG("PREFETCH ABORT: %#x", lr);

  while(1){
    gpio_pulse(LED_PIN, 3);
    sys_timer_sleep(1000000);
  }
}

void data_abort_handler(uint32_t lr, uint32_t dfsr, uint32_t sp){
  asm inline("cpsie i");
  SYS_LOG("DATA ABORT: %#x, %#x, %#x", lr, dfsr, sp);
  while(1){
    gpio_pulse(LED_PIN, 4);
    sys_timer_sleep(1000000);
  }
}

void __attribute__((interrupt("IRQ"))) interrupt_handler(void){
  if(!(AUX->MINI_UART_IRQ_STATUS & 0b001)){
    uint8_t irq_status = AUX->MINI_UART_IRQ_STATUS;
    uart_irq_handler(irq_status);
    uint32_t rx_size;
    uart_get_rx_size(&rx_size);
    if(irq_status & AUX_MINI_UART_IRQ_STATUS_RECEIVE_IRQ){
      kernel_read_queue_update(STDIN_FILENO, rx_size);
    }
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
