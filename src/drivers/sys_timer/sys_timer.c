
#include "bcm2835.h"

void sys_timer_sleep(uint32_t microseconds){
  
  SYS_TIMER->C0 = SYS_TIMER->CLO + microseconds;
  while(!(SYS_TIMER->CS & 0x01));
  SYS_TIMER->CS &= ~(0x01);
}

uint64_t sys_uptime(void){
  uint64_t clock_high = SYS_TIMER->CHI;
  return (clock_high << 32) + SYS_TIMER->CLO;
}
