#include "bcm2835.h"
#include "uart.h"

#define DLAB_ACCESS 7U
#define BAUDRATE 115200U
#define SYSFREQ (250000000UL)
#define UART_TX_PIN GPIO14
#define UART_RX_PIN GPIO15

status_t uart_init(){

  AUX->ENABLES = 0b1; // enable uart 
  AUX->MU_IER = 0;
  AUX->MU_CNTL = 0;

  AUX->MU_LCR |= 0b11;

  AUX->MU_MCR = 0;

  AUX->MU_IER = 0;

  AUX->MU_IIR = 0xC6;

  AUX->MU_BAUD = (SYSFREQ / ( 8 * BAUDRATE )) - 1;

  gpio_func(UART_TX_PIN, GPIO_ALT_FUNC5);
  gpio_func(UART_RX_PIN, GPIO_ALT_FUNC5);



  GPIO->PUD = 0;
  for( volatile int i=0; i<150; i++ ) { }
  GPIO->PUDCLK[0] = ( 1 << 14 );
  for( volatile int i=0; i<150; i++ ) { }
  GPIO->PUDCLK[0] = 0;

  AUX->MU_CNTL = 0b10;

  return STATUS_OK;
}

status_t uart_send(){
  AUX->MU_IO = 'h';
  return STATUS_OK;
}

