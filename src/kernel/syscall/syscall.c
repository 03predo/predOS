#include <errno.h>
#undef errno
extern int errno;

#include <sys/stat.h>
#include <sys/times.h>

#include "kernel.h"

#define HEAP_ADDR 0x1f000000

char *__env[1] = {0};
char **environ = __env;

int _open(const char *pathname, int flags){
  asm inline("SVC "XSTR(SVC_OPEN));
  int fd = -1;
  GET_R0(fd);
  return fd;
}

int _close(int file){
  asm inline("SVC "XSTR(SVC_CLOSE)); 
  int fd = -1;
  GET_R0(fd);
  return fd;
}

int _read(int file, char *ptr, int len){
  asm inline("SVC "XSTR(SVC_READ)); 
  int bytes_read = -1;
  GET_R0(bytes_read);
  return bytes_read;
}

int _write(int file, char *ptr, int len){
  asm inline("SVC "XSTR(SVC_WRITE)); 
  int bytes_written = -1;
  GET_R0(bytes_written);
  return bytes_written;
}

int _lseek(int file, int offset, int whence){
  asm inline("SVC "XSTR(SVC_LSEEK)); 
  int new_offset = -1;
  GET_R0(new_offset);
  return new_offset;
}

int execv(const char *pathname, char *const argv[]){
  asm inline("SVC "XSTR(SVC_EXECV)); 
  return -1;
}

int _execve(const char *filename, char *const argv[], char *const envp[]){
  asm inline("SVC "XSTR(SVC_EXECV));
  return -1;
}

void _exit(int status){
  asm inline("SVC "XSTR(SVC_EXIT)); 
  while(1);
}

int _fork(){
  asm inline("SVC "XSTR(SVC_FORK));
  int pid = -1;
  GET_R0(pid);
  return pid;
}

int _wait(){
  return -1;
}

int yield(){
  asm inline("SVC "XSTR(SVC_YIELD));
  return 0;
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
  static char* heap_end = (char*)HEAP_ADDR;
  char* prev_heap_end;

  if( heap_end == 0 ) heap_end = &_end;

  prev_heap_end = heap_end;
  heap_end += incr;

  return (caddr_t)prev_heap_end; 
}

void outbyte(char b){
}
