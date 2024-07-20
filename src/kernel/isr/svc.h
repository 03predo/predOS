#pragma once

#include <stdint.h>
#include "status.h"

status_t svc_handler(uint32_t* sp, uint32_t svc);
