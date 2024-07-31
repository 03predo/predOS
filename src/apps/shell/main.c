#include <stdio.h>

int main(){
  printf("starting shell\n");
  char c;
  int ret = scanf("%c", c);
  if(ret = EOF){
    printf("scanf failed\n");
    return -1;
  }
  printf("got char: %c\n", c);
  return 0;
}
