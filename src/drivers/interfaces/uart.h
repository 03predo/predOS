#pragma once

#include "status.h"
#include "gpio.h"

status_t uart_init();
status_t uart_send(char c);
status_t uart_print(char* s);
