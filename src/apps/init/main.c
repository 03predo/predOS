#include <stdio.h>
#include <unistd.h>


int main(){
  printf("init\n");
  pid_t child_pid = fork();
  if(child_pid == -1){
    printf("fork failed\n");
    return -1;
  }else if(child_pid == 0){
    while(1){
      printf("child 1\n");
      usleep(1000000);
    }
  }else{
    printf("parent: %d\n", child_pid);
  }

  child_pid = fork();
  if(child_pid == -1){
    printf("fork failed\n");
    return -1;
  }else if(child_pid == 0){
    while(1){
      printf("child 2\n");
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
