#pragma once

typedef enum {
  STATUS_OK = 0,
  STATUS_ERR = 1
}status_t;

#define STATUS_OK_OR_RETURN(code)          \
  ({                                       \
    __typeof__(code) status_expr = (code); \
    if (status_expr) return status_expr;   \
  })
