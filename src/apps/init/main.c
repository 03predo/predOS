#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>


int main(){
  printf("init\n");
  pid_t child_pid = fork();
  if(child_pid == -1){
    printf("fork failed\n");
    return -1;
  }else if(child_pid == 0){
    char *args[] = {"echo", "hello world", NULL};
    execv(args[0], args);
  }else{
    printf("parent: %d\n", child_pid);
    int status = -1;
    pid_t child_pid = wait(&status);
    printf("child_pid: %d, status: %d\n", child_pid, status);
  }

  child_pid = fork();
  if(child_pid == -1){
    printf("fork failed\n");
    return -1;
  }else if(child_pid == 0){
    char *args[] = {"shell", NULL};
    execv(args[0], args);
  }

  while(1) usleep(0);
  return 0;
}
