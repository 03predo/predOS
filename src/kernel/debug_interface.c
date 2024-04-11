#include "debug_interface.h"
#include "sys_log.h"

void debug_event_handler(uint32_t* link_register){
  uint32_t bkpt_id = link_register[-1];
  bkpt_id = (bkpt_id & 0xf) + ((bkpt_id & 0xfff00) >> 4);
  switch(bkpt_id){
    default:
      SYS_LOG("breakpoint: %#x", bkpt_id);
      break;
  }
}
