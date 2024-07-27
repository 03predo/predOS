#include <stdio.h>
#include <unistd.h>

int main(){
  printf("init\n");
  fork();
  while(1);
  return 0;
}
