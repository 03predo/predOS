#include "bcm2835.h"
#include "gpio.h"

#define SEL_FIELD_WIDTH 3
#define SEL_REG_WIDTH 30

#define SET_REG_WIDTH 32


status_t gpio_func(gpio_pin_t pin, gpio_func_t func){
  uint8_t reg_indx = (pin * SEL_FIELD_WIDTH) / SEL_REG_WIDTH;
  uint8_t field_indx = (pin * SEL_FIELD_WIDTH) % SEL_REG_WIDTH;
  uint8_t mask = 0;
  for(uint8_t i = 0; i < SEL_FIELD_WIDTH; ++i) mask |= 1 << i;
  GPIO->SEL[reg_indx] &= ~(mask << field_indx);
  GPIO->SEL[reg_indx] |= (func << field_indx);
  return STATUS_OK;
}

status_t gpio_set(gpio_pin_t pin){
  uint8_t reg_indx = pin / SET_REG_WIDTH;
  uint8_t mask = 0;
  GPIO->SET[reg_indx] |= (1U << pin); 
  return STATUS_OK;
}

status_t gpio_clear(gpio_pin_t pin){
  uint8_t reg_indx = pin / SET_REG_WIDTH;
  uint8_t mask = 0;
  GPIO->CLR[reg_indx] |= (1U << pin); 
  return STATUS_OK;
}

