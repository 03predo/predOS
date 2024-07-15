#include <errno.h>
#undef errno
extern int errno;

#include <sys/stat.h>
#include <sys/times.h>

#include "uart.h"
#include "sys_log.h"
#include "fat.h"

char *__env[1] = {0};
char **environ = __env;

void _exit(int status){
  (void)status;
  while(1);
}

int _close(int file){
  if(fat_close_file(file) == STATUS_OK){
    return 0;
  }
  return -1;
}

int _lseek(int file, int ptr, int dir){
  return 0;
}

int _read(int file, char *ptr, int len){
  int bytes_read = -1;
  if(fat_read_file(file, ptr, len, &bytes_read) != STATUS_OK){
    return -1;
  }
  return bytes_read;
}

int _isatty(int file){
  return 1;
}

int _fstat(int file, struct stat *st){
  st->st_mode = S_IFCHR;
  return 0;
}

caddr_t _sbrk(int incr){
  extern char _end;
  static char* heap_end = 0;
  char* prev_heap_end;

  if( heap_end == 0 ) heap_end = &_end;

  prev_heap_end = heap_end;
  heap_end += incr;

  return (caddr_t)prev_heap_end; 
}

void outbyte(char b){
}

int _write(int file, char *ptr, int len){
  uart_print(ptr, len);
  return len;
}

int _open(const char *pathname, int flags){
  int fd = -1;
  fat_open_file(pathname, flags, &fd); 
  return fd;
}

