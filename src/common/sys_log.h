#pragma once
#include <stdio.h>

#include "log_level.h"
#include "sys_timer.h"
#include "uart.h"

#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_VERBOSE
#endif

#ifdef TEST
#define SYS_LOG(X, ...) printf("(%012lu) <%s:%d> "X"\r\n", sys_uptime(), __func__, __LINE__ __VA_OPT__(,) __VA_ARGS__)
#else
#define SYS_LOG(X, ...) \
{ \
  char _buf[500]; \
  int _n = sprintf(_buf, "(%012llu) <%s:%d> "X"\r\n", sys_uptime(), __func__, __LINE__ __VA_OPT__(,) __VA_ARGS__); \
  if(_n == -1) uart_print("sprintf failed", 14); \
  uart_print(_buf, _n); \
}
#endif

#define SYS_LOGD(X, ...) if(LOG_LEVEL <= LOG_LEVEL_DEBUG) SYS_LOG(X, __VA_ARGS__)
#define SYS_LOGV(X, ...) if(LOG_LEVEL <= LOG_LEVEL_VERBOSE) SYS_LOG(X, __VA_ARGS__)
#define SYS_LOGE(X, ...) if(LOG_LEVEL <= LOG_LEVEL_ERROR) SYS_LOG(X, __VA_ARGS__)

