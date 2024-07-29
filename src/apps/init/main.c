#include <stdio.h>
#include <unistd.h>

#define GET_FP(fp) asm inline ("mov %0, fp" : "=r" (fp) : : )

int main(){
  printf("init\n");
  pid_t child_pid = fork();
  uint32_t fp = 0;
  GET_FP(fp);
  if(child_pid == -1){
    printf("fork failed\n");
    return -1;
  }if(child_pid == 0){
    while(1){
      printf("child\n");
      usleep(1000000);
    } 
  }else{
    while(1){
      printf("parent: %d\n", child_pid);
      usleep(1000000);
    }
  }
  while(1);
  return 0;
}
