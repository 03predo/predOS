#include "example.h"
#include "sys_log.h"
#include "util.h"

int main(){
  uint32_t counter = 0;
  while(1){
    SYS_LOG("counter: %d", counter++);
    sys_timer_sleep(1000000);  
  }
  return 0;
}
