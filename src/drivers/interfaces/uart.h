#pragma once

#include "status.h"
#include "gpio.h"

status_t uart_init(uint32_t baudrate);
status_t uart_send(char c);
status_t uart_get_rx_size(uint32_t* rx_size);
status_t uart_read(char* buf, int len, int* bytes_read);
status_t uart_print(char* c, int len);
status_t uart_irq_handler(uint8_t irq_status);

