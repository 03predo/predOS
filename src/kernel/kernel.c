#include <stdint.h>
#include <stdio.h>
#include "gpio.h"
#include "sys_timer.h"
#include "arm_timer.h"
#include "uart.h"

#define LED_PIN 16

extern void _enable_interrupts(void);

int kernel_main(){
  gpio_func(LED_PIN, GPIO_OUTPUT); 
  arm_timer_init(0x400);
  _enable_interrupts();

  uart_init();
  printf("hello from predOS\r\n");
  
  uint32_t counter = 0;
  while(1){
    printf("counter = %d\r\n", counter);

    sys_timer_sleep(1000000);  

    counter++;
  }

  return 0;
}
