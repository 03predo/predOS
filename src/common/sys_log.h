#pragma once
#include <stdio.h>
#include "sys_timer.h"

#define SYS_LOG(X, ...) printf("(%012llu) <%s:%d> "X"\r\n", sys_uptime(), __func__, __LINE__ __VA_OPT__(,) __VA_ARGS__)
