#pragma once
#include <stdio.h>

#include "log_level.h"

#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_INFO
#endif

#define APP_LOGD(X, ...) if(LOG_LEVEL <= LOG_LEVEL_DEBUG) printf(X"\r\n" __VA_OPT__(,) __VA_ARGS__)
#define APP_LOGI(X, ...) if(LOG_LEVEL <= LOG_LEVEL_INFO) printf(X"\r\n" __VA_OPT__(,) __VA_ARGS__)
#define APP_LOGE(X, ...) if(LOG_LEVEL <= LOG_LEVEL_ERROR) printf(X"\r\n" __VA_OPT__(,) __VA_ARGS__)



