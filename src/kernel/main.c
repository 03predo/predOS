#include "stdint.h"
#include "gpio.h"

#define LED_PIN 16

int main(void) __attribute__((naked));
int main(void)
{
    gpio_func(LED_PIN, GPIO_OUTPUT); 

    while(1)
    {
        for(uint32_t tim = 0; tim < 5000000; tim++);

        gpio_clear(LED_PIN);

        for(uint32_t tim = 0; tim < 5000000; tim++);

        gpio_set(LED_PIN);
    }
}
