#include "svc.h"
#include "sys_log.h"
#include "kernel.h"
#include "fat.h"

static status_t open(uint32_t* sp){
  char* file_name = (char*)sp[0];
  int flags = sp[1];
  int fd = -1;
  SYS_LOGD("pathname: %s, flags: %#x", file_name, flags);
  if(fat_open_file(file_name, flags, &fd) != STATUS_OK){
    SYS_LOGD("fat_open_file failed");
  }
  sp[0] = fd;
  return STATUS_OK;
}

static status_t close(uint32_t* sp){
  int fd = sp[0];
  SYS_LOGD("fd: %#x", fd);
  if(fat_close_file(fd) != STATUS_OK){
    SYS_LOGD("fat_close_file failed");
    sp[0] = -1;
  }else{
    sp[0] = 0;
  }
  return STATUS_OK;
}

static status_t read(uint32_t* sp){
  int fd = sp[0];
  char* buf = (char*)sp[1];
  int len = sp[2];
  int bytes_read = 0;
  SYS_LOGD("fd: %#x, buf: %#x, len: %#x", fd, buf, len);
  if(fat_read_file(fd, buf, len, &bytes_read) != STATUS_OK){
    SYS_LOGD("fat_open_file failed");
  }
  sp[0] = bytes_read;
  return STATUS_OK;
}

static status_t write(uint32_t* sp){
  int fd = sp[0];
  char* buf = (char*)sp[1];
  int len = sp[2];
  SYS_LOGD("fd: %#x, buf: %#x, len: %#x", fd, buf, len);

  if((fd >= 0) && (fd < 3)){
    uart_print(buf, len);
    sp[0] = len;
    return STATUS_OK;
  }

  int bytes_written = 0;
  if(fat_write_file(fd, buf, len, &bytes_written) != STATUS_OK){
    SYS_LOGD("fat_open_file failed");
  }
  sp[0] = bytes_written;
  return STATUS_OK;
}

status_t svc_handler(uint32_t* sp, uint32_t svc){
  SYS_LOGD("sp: %#x, svc: %#x", sp, svc);
  switch(svc){
    case SVC_OPEN: 
      STATUS_OK_OR_RETURN(open(sp));
      break;
    case SVC_CLOSE:
      STATUS_OK_OR_RETURN(close(sp));
      break;
    case SVC_READ:
      STATUS_OK_OR_RETURN(read(sp));
      break;
    case SVC_WRITE:
      STATUS_OK_OR_RETURN(write(sp));
      break;
    default:
      SYS_LOGE("undefined svc: %#x", svc);
      return STATUS_ERR;
  }
  return STATUS_OK;
}
