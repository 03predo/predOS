#include "svc.h"
#include "sys_log.h"
#include "kernel.h"
#include "fat.h"

static status_t svc_open(uint32_t* sp){
  char* file_name = (char*)sp[0];
  int flags = sp[1];
  SYS_LOGD("pathname: %s, flags: %#x", file_name, flags);
  sp[0] = kernel_open(file_name, flags);
  return STATUS_OK;
}

static status_t svc_close(uint32_t* sp){
  int fd = sp[0];
  SYS_LOGD("fd: %#x", fd);
  sp[0] = kernel_close(fd);
  return STATUS_OK;
}

static status_t svc_read(uint32_t* sp){
  int fd = sp[0];
  char* buf = (char*)sp[1];
  int len = sp[2];
  int bytes_read = 0;
  SYS_LOGD("fd: %#x, buf: %#x, len: %#x", fd, buf, len); 
  sp[0] = kernel_read(fd, buf, len);
  return STATUS_OK;
}

static status_t svc_write(uint32_t* sp){
  int fd = sp[0];
  char* buf = (char*)sp[1];
  int len = sp[2];
  SYS_LOGD("fd: %#x, buf: %#x, len: %#x", fd, buf, len);
  sp[0] = kernel_write(fd, buf, len);
  return STATUS_OK;
}

static status_t svc_lseek(uint32_t* sp){
  int fd = sp[0];
  int offset = sp[1];
  int whence = sp[2];
  SYS_LOGD("fd: %#x, offset: %#x, whence: %#x", fd, offset, whence); 
  sp[0] = kernel_lseek(fd, offset, whence);
  return STATUS_OK;
}

static status_t svc_execv(uint32_t* sp){
  const char* pathname = (const char*)sp[0];
  char* const* argv = (char* const*)sp[1];
  sp[0] = kernel_execv(pathname, argv);
  return STATUS_OK;
}

static status_t svc_exit(uint32_t* sp){
  int exit_status = sp[0];
  kernel_exit(exit_status);
  return STATUS_OK;
}

static status_t svc_fork(uint32_t* sp){
  sp[0] = kernel_fork();
  return STATUS_OK;
}

static status_t svc_yield(uint32_t* sp){
  sp[0] = kernel_yield();
  return STATUS_OK;
}

static status_t svc_wait(uint32_t* sp){
  SYS_LOGD("exit_status: %#x", sp[0]);
  kernel_wait();
  return STATUS_OK;
}

static status_t svc_usleep(uint32_t* sp){
  uint32_t timeout = sp[0];
  sp[0] = kernel_usleep(timeout);
  return STATUS_OK;
}

static status_t svc_signal(uint32_t* sp){
  int signum = (int)sp[0];
  sig_t handler = (sig_t)sp[1];
  sp[0] = (uint32_t)kernel_signal(signum, handler);
  return STATUS_OK;
}

static status_t svc_raise(uint32_t* sp){
  int signum = (int)sp[0];
  sp[0] = kernel_raise(signum);
  return STATUS_OK;
}

static status_t svc_kill(uint32_t* sp){
  pid_t pid = (pid_t)sp[0];
  int signum = (int)sp[1];
  sp[0] = kernel_kill(pid, signum);
  return STATUS_OK;
}

static status_t svc_led(uint32_t* sp){
  int on = (int)sp[0];
  sp[0] = kernel_led(on);
  return STATUS_OK;
}

static status_t svc_ps(uint32_t* sp){
  sp[0] = kernel_ps();
  return STATUS_OK;
}

status_t svc_handler(uint32_t* sp, uint32_t svc){
  SYS_LOGD("sp: %#x, svc: %#x", sp, svc);
  switch(svc){
    case SVC_OPEN: 
      STATUS_OK_OR_RETURN(svc_open(sp));
      break;
    case SVC_CLOSE:
      STATUS_OK_OR_RETURN(svc_close(sp));
      break;
    case SVC_READ:
      STATUS_OK_OR_RETURN(svc_read(sp));
      break;
    case SVC_WRITE:
      STATUS_OK_OR_RETURN(svc_write(sp));
      break;
    case SVC_LSEEK:
      STATUS_OK_OR_RETURN(svc_lseek(sp));
      break;
    case SVC_EXECV:
      STATUS_OK_OR_RETURN(svc_execv(sp));
      break;
    case SVC_EXIT:
      STATUS_OK_OR_RETURN(svc_exit(sp));
      break;
    case SVC_FORK:
      STATUS_OK_OR_RETURN(svc_fork(sp));
      break;
    case SVC_YIELD:
      STATUS_OK_OR_RETURN(svc_yield(sp));
      break;
    case SVC_WAIT:
      STATUS_OK_OR_RETURN(svc_wait(sp));
      break;
    case SVC_USLEEP:
      STATUS_OK_OR_RETURN(svc_usleep(sp));
      break;
    case SVC_SIGNAL:
      STATUS_OK_OR_RETURN(svc_signal(sp));
      break;
    case SVC_RAISE:
      STATUS_OK_OR_RETURN(svc_raise(sp));
      break;
    case SVC_KILL:
      STATUS_OK_OR_RETURN(svc_kill(sp));
      break;
    case SVC_LED:
      STATUS_OK_OR_RETURN(svc_led(sp));
      break;
    case SVC_PS:
      STATUS_OK_OR_RETURN(svc_ps(sp));
      break;
    default:
      SYS_LOGE("undefined svc: %#x", svc);
      return STATUS_ERR;
  }
  return STATUS_OK;
}
