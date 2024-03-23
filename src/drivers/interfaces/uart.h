#pragma once

#include "status.h"
#include "gpio.h"

status_t uart_init(uint32_t baudrate);
status_t uart_send(char c);

