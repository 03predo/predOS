#include "bcm2835.h"

gpio_t* GPIO = (gpio_t*)(PERIPHERAL_BASE + GPIO_OFFSET);
aux_t* AUX = (aux_t*)(PERIPHERAL_BASE + AUX_OFFSET);
sys_timer_t* SYS_TIMER = (sys_timer_t*)(PERIPHERAL_BASE + SYS_TIMER_OFFSET);
