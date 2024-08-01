#include <stdio.h>
#include <unistd.h>

#define MAX_COMMAND_SIZE 1024
#define MAX_ARG_NUM 10
#define SHELL_TOKEN "predOS# "

char cmd[MAX_COMMAND_SIZE];
char* args[MAX_ARG_NUM + 1]; // +1 for NULL


void cmd_handler(char* cmd, uint32_t size);

int main(){
  uint8_t c = 0;
  uint32_t index = 0; 
   
  write(STDOUT_FILENO, SHELL_TOKEN, sizeof(SHELL_TOKEN));
  while(read(STDIN_FILENO, &c, 1) != -1){
    if(c == '\b'){
      write(STDOUT_FILENO, "\b \b", 3);
      index--;
    }else if(c == '\r'){
      write(STDOUT_FILENO, "\r\n", 2);
      cmd[index] = '\0';
      cmd_handler(cmd, index);
      write(STDOUT_FILENO, SHELL_TOKEN, sizeof(SHELL_TOKEN));
      index = 0;
    }else{
      write(STDOUT_FILENO, &c, 1);
      cmd[index] = c;
      index++;
      if(index >= MAX_COMMAND_SIZE){
        printf("\r\ncommand too long\r\n");
        index = 0;
      }
    }
  }  

  printf("read failed\n"); 
  return -1;
}

void cmd_handler(char* cmd, uint32_t size){
  printf("cmd: %s\r\n", cmd);
  uint32_t arg_num = 0;
  args[0] = cmd;
  for(uint32_t i = 0; i < size; ++i){
    if(cmd[i] == ' '){
      if((arg_num + 1) < MAX_ARG_NUM){
        arg_num++;
        cmd[i] = '\0';
        args[arg_num] = &cmd[i + 1];
      }
    }
  }

  for(uint32_t i = 0; i <= arg_num; ++i){
    printf("args[%d]: %s\r\n", i, args[i]);
  }
}
