#include "example.h"
#include "sys_log.h"
#include "util.h"

void example_main(uint32_t r0){
  uint32_t lr;
  GET_LR(lr);
  uint32_t cpsr;
  GET_CPSR(cpsr);
  SYS_LOG("starting example app: r0 = %#x, lr = %#x, cpsr = %#x", r0, lr, cpsr);
  uint32_t counter = 0;
  while(1){
    SYS_LOG("counter: %d", counter++);
    sys_timer_sleep(1000000);  
  }
}
