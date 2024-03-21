#include "stdint.h"
#include "gpio.h"
#include "sys_timer.h"
#include "uart.h"

#define LED_PIN 16

int kernel_main(){
  uart_init();
  gpio_func(LED_PIN, GPIO_OUTPUT); 

  while(1){
    //uart_send();
    
    gpio_set(LED_PIN);
    sys_timer_sleep(500000);

    gpio_clear(LED_PIN);
    sys_timer_sleep(500000);
  }
  return 0;
}
