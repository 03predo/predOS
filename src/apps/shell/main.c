#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#include "app_log.h"

#define MAX_COMMAND_SIZE 1024
#define MAX_ARG_NUM 10
#define SHELL_TOKEN "predOS# "


void cmd_handler(char* cmd, uint32_t size);

int main(){
  uint8_t c = 0;
  uint32_t index = 0; 
  char cmd[MAX_COMMAND_SIZE];
   
  write(STDOUT_FILENO, SHELL_TOKEN, sizeof(SHELL_TOKEN));
  while(read(STDIN_FILENO, &c, 1) != -1){
    if(c == '\b'){
      if(index > 0){
        write(STDOUT_FILENO, "\b \b", 3);
        index--;
      }
    }else if(c == '\r'){
      write(STDOUT_FILENO, "\r\n", 2);
      cmd[index] = '\0';
      APP_LOGD("CMD: %s", cmd);
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
  char* args[MAX_ARG_NUM + 1]; // +1 for NULL
  APP_LOGD("cmd: %s", cmd);
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
    APP_LOGD("args[%d]: %s", i, args[i]);
  }

  args[arg_num + 1] = NULL;
  pid_t child_pid = fork();
  if(child_pid == -1){
    APP_LOGE("fork failed");
    return;
  }else if(child_pid == 0){
    if(execv(args[0], args) == -1){
      exit(-1);
    }
  }else{
    int status = -1;
    pid_t child_pid = wait(&status);
    APP_LOGI("exit status: %d", status);
  }  
}
