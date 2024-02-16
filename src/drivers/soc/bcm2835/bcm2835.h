#pragma once

#include <stdint.h>

#define PERIPHERAL_BASE 0x20000000UL

/*
 * GPIO
 */

#define GPIO_BASE         0x200000UL

typedef struct {
  uint32_t SEL[6];
  uint32_t res0;
  uint32_t SET[2];
  uint32_t res1;
  uint32_t CLR[2];
}gpio_t;

gpio_t* GPIO = (gpio_t*)(PERIPHERAL_BASE + GPIO_BASE);
