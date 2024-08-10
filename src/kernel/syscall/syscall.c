#include <errno.h>
#undef errno
extern int errno;

#include <sys/stat.h>
#include <sys/times.h>
#include <signal.h>

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

int _wait(int* status){
  asm inline("SVC "XSTR(SVC_WAIT));
  int pid = -1;
  GET_R0(pid);
  int exit_status = -1;
  GET_R1(exit_status);
  *status = exit_status;
  return pid;
}

int yield(){
  asm inline("SVC "XSTR(SVC_YIELD));
  return 0;
}

int usleep(useconds_t usec){
  asm inline("SVC "XSTR(SVC_USLEEP));
  int status = -1;
  GET_R0(status);
  return status;
}

sig_t __wrap_signal(int signum, sig_t handler){
  asm inline("SVC "XSTR(SVC_SIGNAL));
  sig_t prev_handler = NULL;
  GET_R0(prev_handler);
  return prev_handler;
}

int __wrap_raise(int signum){
  asm inline("SVC "XSTR(SVC_RAISE));
  int ret = -1;
  GET_R0(ret);
  return ret;
}

int _kill(pid_t pid, int sig){
  asm inline("SVC "XSTR(SVC_KILL));
  int ret = -1;
  GET_R0(ret);
  return ret;
}

int led(int on){
  asm inline("SVC "XSTR(SVC_LED));
  int ret = -1;
  GET_R0(ret);
  return ret;
}

int ps(){
  asm inline("SVC "XSTR(SVC_PS));
  int ret = -1;
  GET_R0(ret);
  return ret;
}

pid_t _getpid(void){
  return -1;
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
