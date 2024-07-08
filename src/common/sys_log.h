#pragma once
#include <stdio.h>

#include "log_level.h"
#include "sys_timer.h"

#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_VERBOSE
#endif

#define SYS_LOG(X, ...) printf("(%012llu) <%s:%d> "X"\r\n", sys_uptime(), __func__, __LINE__ __VA_OPT__(,) __VA_ARGS__)
#define SYS_LOGD(X, ...) if(LOG_LEVEL <= LOG_LEVEL_DEBUG) SYS_LOG(X, __VA_ARGS__)
#define SYS_LOGV(X, ...) if(LOG_LEVEL <= LOG_LEVEL_VERBOSE) SYS_LOG(X, __VA_ARGS__)
#define SYS_LOGE(X, ...) if(LOG_LEVEL <= LOG_LEVEL_ERROR) SYS_LOG(X, __VA_ARGS__)

