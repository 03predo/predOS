#include "bcm2835.h"
#include "uart.h"

#define DLAB_ACCESS 7U
#define BAUDRATE 115200U
#define SYSFREQ (250000000UL)
#define UART_TX_PIN GPIO14
#define UART_RX_PIN GPIO15
#define UART_MULSR_TX_EMPTY ( 1 << 5 )

status_t uart_init(){

  AUX->ENABLES = 0b1; // enable uart 
  AUX->MINI_UART_IRQ_ENABLE = 0;
  AUX->MINI_UART_CONTROL = 0;

  AUX->MINI_UART_LINE_CONTROL |= 0b11;

  AUX->MINI_UART_MODEM_CONTROL = 0;

  AUX->MINI_UART_IRQ_ENABLE = 0;

  AUX->MINI_UART_IRQ_STATUS = 0xC6;

  AUX->MINI_UART_BAUDRATE = (SYSFREQ / ( 8 * BAUDRATE )) - 1;

  gpio_func(UART_TX_PIN, GPIO_ALT_FUNC5);
  gpio_func(UART_RX_PIN, GPIO_ALT_FUNC5);



  GPIO->PUD = 0;
  for( volatile int i=0; i<150; i++ ) { }
  GPIO->PUDCLK[0] = ( 1 << 14 );
  for( volatile int i=0; i<150; i++ ) { }
  GPIO->PUDCLK[0] = 0;

  AUX->MINI_UART_CONTROL = 0b10;

  return STATUS_OK;
}

status_t uart_send(char c){
  while(!(AUX->MINI_UART_LINE_STATUS & UART_MULSR_TX_EMPTY));
  AUX->MINI_UART_IO = c;
  return STATUS_OK;
}

status_t uart_print(char* s){
  int i = 0;
  while(s[i] != '\0'){
    uart_send(s[i]);
  }
  return STATUS_OK;
}


