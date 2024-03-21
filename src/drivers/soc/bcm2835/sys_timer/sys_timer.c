
#include "bcm2835.h"

void sys_timer_sleep(uint32_t microseconds){
  volatile uint32_t timestamp = SYS_TIMER->CLO;
  while((SYS_TIMER->CLO - timestamp) < microseconds && (SYS_TIMER->CLO > timestamp));
}
