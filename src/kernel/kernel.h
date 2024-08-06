#pragma once

#include "status.h"
#include "util.h"

#define SVC_OPEN    0x00
#define SVC_CLOSE   0x01
#define SVC_READ    0x02
#define SVC_WRITE   0x03
#define SVC_LSEEK   0x04
#define SVC_EXECV   0x05
#define SVC_EXIT    0x06
#define SVC_FORK    0x07
#define SVC_WAIT    0x08
#define SVC_YIELD   0x09
#define SVC_USLEEP  0x0a

status_t kernel_context_save(uint32_t* sp);
status_t kernel_context_switch();
status_t kernel_read_queue_update(int fd, uint32_t size);

int kernel_open(const char *pathname, int flags);
int kernel_close(int file);
int kernel_read(int file, char *ptr, int len);
int kernel_write(int file, char *ptr, int len);
int kernel_lseek(int file, int offset, int whence);
int kernel_execv(const char *pathname, char *const argv[]);
void kernel_exit(int exit_status);
int kernel_fork();
int kernel_wait();
int kernel_yield();
int kernel_usleep(uint32_t timeout);

