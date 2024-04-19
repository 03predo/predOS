#include "power_management.h"
#include "bcm2835.h"

status_t power_management_reset(){
  *WATCHDOG = 10 | PM_PASSWORD;
  uint32_t val = *(RESET_CONTROLLER);
  val &= PM_WRCFG_CLR;
  val |= PM_PASSWORD | PM_FULL_RESET;
  *RESET_CONTROLLER = val;
}

