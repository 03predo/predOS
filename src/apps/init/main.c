#include <stdio.h>
#include <unistd.h>

int yield();

int main(){
  printf("init\n");
  pid_t child_pid = fork();
  if(child_pid == -1){
    printf("fork failed\n");
    return -1;
  }if(child_pid == 0){
    printf("child\n");
    yield();
    printf("child again\n");
  }else{
    printf("parent: %d\n", child_pid);
    yield();
    printf("parent again\n", child_pid);
    yield();
  }
  while(1);
  return 0;
}
