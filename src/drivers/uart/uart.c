#include <string.h>
#include "bcm2835.h"
#include "uart.h"
#include "sys_log.h"
#include "power_management.h"
#include "emmc.h"
#include "gpio.h"


#define UART_TX_PIN GPIO14
#define UART_RX_PIN GPIO15
#define UART_TX_BUFFER_SIZE 1024
#define UART_RX_BUFFER_SIZE 1024

#define LED_PIN 16

typedef enum {
  STANDBY = 0, 
  IMAGE_HEADER,
  IMAGE_PAYLOAD
} uart_state_t;

typedef union {
  struct {
    uint16_t image_start_address;
    uint32_t image_size;
  } fields;
  uint8_t raw[6];
} uart_image_header;

typedef struct {
  char tx_buf[UART_TX_BUFFER_SIZE];
  uint32_t tx_indx;
  uint32_t tx_size;

  char rx_buf[UART_RX_BUFFER_SIZE];
  uint32_t rx_indx;
  uint32_t rx_size;

  uart_state_t state;

  uint32_t received_blocks;
  uint32_t received_bytes;
  uart_image_header image_header;
  emmc_block_t block;
} uart_driver;

static uart_driver ud;

status_t uart_init(uint32_t baudrate){
  // struct initialization
  for(int i = 0; i < UART_TX_BUFFER_SIZE; ++i){
    ud.tx_buf[i] = 0;
  }
  ud.tx_indx = 0;

  for(int i = 0; i < UART_RX_BUFFER_SIZE; ++i){
    ud.rx_buf[i] = 0;
  }
  ud.rx_indx = 0;

  // register initialization
  AUX->ENABLES |= AUX_IRQ_MINI_UART; // enable uart 

  AUX->MINI_UART_IRQ_ENABLE = 0;
  AUX->MINI_UART_CONTROL = 0;
  AUX->MINI_UART_MODEM_CONTROL = 0;

  AUX->MINI_UART_LINE_CONTROL = AUX_MINI_UART_LINE_CONTROL_8BIT;
  AUX->MINI_UART_IRQ_STATUS = AUX_MINI_UART_IRQ_STATUS_CLEAR_FIFO;
  AUX->MINI_UART_BAUDRATE = (SYSFREQ / ( 8 * baudrate )) - 1;

  gpio_func(UART_TX_PIN, GPIO_ALT_FUNC5);
  gpio_func(UART_RX_PIN, GPIO_ALT_FUNC5);
  gpio_pud(UART_TX_PIN, PULL_UP_DOWN_DISABLE);
  gpio_pud(UART_RX_PIN, PULL_UP_DOWN_DISABLE);

  AUX->MINI_UART_CONTROL = AUX_MINI_UART_CONTROL_TX_ENABLE | AUX_MINI_UART_CONTROL_RX_ENABLE;

  AUX->MINI_UART_IRQ_ENABLE = AUX_MINI_UART_CONTROL_RX_ENABLE;
  IRQ_CONTROLLER->ENABLE_IRQ1 = (1 << 29);

  ud.state = STANDBY;
  printf("\n");
  return STATUS_OK;
}

status_t uart_handle_command(char* cmd, uint32_t len){
  if(len == 1 && cmd[0] == 'R'){
    SYS_LOG("reseting...");
    power_management_reset();
  }else if(len == 1 && cmd[0] == 'L'){
    ud.state = IMAGE_HEADER;
    SYS_LOG("ready to load image %d", ud.state);
    ud.received_blocks = 0;
    ud.received_bytes = 0;
    ud.image_header.fields.image_size = 0;
  }else if(len == 1 && cmd[0] == 'B'){
    emmc_block_t block;
    SYS_LOG("reading block");
    emmc_read_block(0, 1, &block);
    sys_timer_sleep(1000000);
    emmc_print_block(block);
  }else{
    SYS_LOG("cmd: %s", cmd);
  }
}

status_t uart_irq_handler(){ 
  static uint8_t lit = 0;
  uint8_t irq_status = AUX->MINI_UART_IRQ_STATUS;
  if(irq_status & AUX_MINI_UART_IRQ_STATUS_RECEIVE_IRQ){ 
    uint8_t c = AUX->MINI_UART_IO;
    switch(ud.state){
      case STANDBY:
        if(c == '\r'){
          uart_print("\n", 1);
          ud.rx_buf[ud.rx_indx + ud.rx_size] = '\0';
          uart_handle_command(&ud.rx_buf[ud.rx_indx], ud.rx_size);
          ud.rx_indx = (ud.rx_indx + ud.rx_size) % UART_RX_BUFFER_SIZE;
          ud.rx_size = 0;
        }else{
          ud.rx_buf[ud.rx_indx + ud.rx_size] = c;
          ud.rx_size = (ud.rx_size + 1) % UART_RX_BUFFER_SIZE;
          uart_print(&c, 1);
        }
        break;
      case IMAGE_HEADER:
        ud.image_header.fields.image_size += (c << ((ud.received_bytes) * 8));
        ud.received_bytes++;
        if(ud.received_bytes >= 4){
          SYS_LOG("image size is %d", ud.image_header.fields.image_size);
          emmc_start_write(0, ud.image_header.fields.image_size);
          ud.received_bytes = 0;
          ud.state = IMAGE_PAYLOAD;
        }
        break;
      case IMAGE_PAYLOAD:
        ud.block.buf[ud.received_bytes] = c;
        ud.received_bytes++;
        if((ud.received_bytes % 4) == 0){
          uint32_t word = ud.block.buf[ud.received_bytes - 4];
          word += ud.block.buf[ud.received_bytes - 3] << 8;
          word += ud.block.buf[ud.received_bytes - 2] << 16;
          word += ud.block.buf[ud.received_bytes - 1] << 24;
          emmc_write_word(word);
        }
        if(ud.received_bytes == EMMC_BLOCK_SIZE){
          ud.received_bytes = 0;
          ud.received_blocks++;

          if(ud.received_blocks >= ud.image_header.fields.image_size){
            ud.state = STANDBY;

            emmc_finish_write();
            SYS_LOG("received image size of %d", ud.received_blocks); 
          }
        }
        break;
    }
    
  }

  if(irq_status & AUX_MINI_UART_IRQ_STATUS_TRANSMIT_IRQ){
    if(ud.tx_size > 0){
      AUX->MINI_UART_IO = ud.tx_buf[ud.tx_indx];
      ud.tx_indx = (ud.tx_indx + 1) % UART_TX_BUFFER_SIZE;
      ud.tx_size--;
    }else{
      AUX->MINI_UART_IRQ_ENABLE &= ~(AUX_MINI_UART_IRQ_ENABLE_TRANSMIT);
    }
  }

  return STATUS_OK;
}



status_t uart_print(char* c, int len){
  for(int i = 0; i < len; i = ++i){
    ud.tx_buf[(i + ud.tx_indx + ud.tx_size) % UART_TX_BUFFER_SIZE] = c[i]; 
  }
  ud.tx_size += len;
  AUX->MINI_UART_IRQ_ENABLE |= AUX_MINI_UART_IRQ_ENABLE_TRANSMIT;
  return STATUS_OK;
}

