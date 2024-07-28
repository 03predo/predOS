#include <stdio.h>
#include <unistd.h>

int main(){
  printf("init\n");
  pid_t child_pid = fork();
  if(child_pid == -1){
    printf("fork failed\n");
    return -1;
  }if(child_pid == 0){
    printf("child\n");
  }else{
    printf("parent: %d\n", child_pid);
  }
  while(1);
  return 0;
}
