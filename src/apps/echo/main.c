#include <stdio.h>
#include <unistd.h>

int main(char *const argv[]){
  uint32_t arg_num = 0;
  while(argv[arg_num] != NULL) arg_num++;
  if(arg_num < 2) return -1;

  for(uint32_t i = 1; i < arg_num; ++i){ 
    printf("%s ", argv[i]);
  }
  printf("\n");
  return 0;
}
