#pragma once

#include "status.h"
#include "gpio_def.h"

status_t gpio_func(gpio_pin_t pin, gpio_func_t func);
status_t gpio_set(gpio_pin_t pin);
status_t gpio_clear(gpio_pin_t pin);
status_t gpio_pud(gpio_pin_t pin, gpio_pud_t mode);

