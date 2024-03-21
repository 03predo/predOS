#include "bcm2835.h"

gpio_t* GPIO = (gpio_t*)(PERIPHERAL_BASE + GPIO_OFFSET);
aux_t* AUX = (aux_t*)(PERIPHERAL_BASE + AUX_OFFSET);
