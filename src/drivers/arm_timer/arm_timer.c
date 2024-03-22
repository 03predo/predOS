#include "arm_timer.h"
#include "bcm2835.h"

#define ARM_TIMER_CTRL_23BIT         ( 1 << 1 )

#define ARM_TIMER_CTRL_PRESCALE_1    ( 0 << 2 )
#define ARM_TIMER_CTRL_PRESCALE_16   ( 1 << 2 )
#define ARM_TIMER_CTRL_PRESCALE_256  ( 2 << 2 )

#define ARM_TIMER_CTRL_INT_ENABLE    ( 1 << 5 )
#define ARM_TIMER_CTRL_INT_DISABLE   ( 0 << 5 )

#define ARM_TIMER_CTRL_ENABLE        ( 1 << 7 )
#define ARM_TIMER_CTRL_DISABLE       ( 0 << 7 )

#define ARM_TIMER_BASIC_IRQ          ( 1 << 0 )

status_t arm_timer_init(uint32_t period){


  ARM_TIMER->LOAD = period;
  ARM_TIMER->CONTROL = ARM_TIMER_CTRL_23BIT |
                       ARM_TIMER_CTRL_ENABLE |
                       ARM_TIMER_CTRL_INT_ENABLE |
                       ARM_TIMER_CTRL_PRESCALE_256;


  IRQ_CONTROLLER->ENABLE_BASIC_IRQ = ARM_TIMER_BASIC_IRQ;

  return STATUS_OK;
}
