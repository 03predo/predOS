#include "stdint.h"
#include "gpio.h"
#include "uart.h"

#define LED_PIN 16

int kernel_init(void) __attribute__((naked)) __attribute__((section(".text.kernel_init")));
int kernel_init(void)
{
  uart_init();
  gpio_func(LED_PIN, GPIO_OUTPUT); 

  while(1){
    uart_send();
    for(uint32_t tim = 0; tim < 5000000; tim++);

    gpio_clear(LED_PIN);

    for(uint32_t tim = 0; tim < 5000000; tim++);

    gpio_set(LED_PIN);
  }
}
