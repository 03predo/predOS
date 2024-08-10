#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>

#include "syscall.h"

#define FILE_NAME "blink.io"

static uint8_t read_ready = 0;

void user_signal_handler(int signum){
  read_ready = 1;
}

int main(char *const argv[]){
  read_ready = 0;
  if(argv[1] == NULL){
    printf("blinkd no args provided\n");
    return -1;
  }
  uint32_t period = atoi(argv[1]);
  printf("%s: setting period to %d\n", argv[0], period);
  sig_t prev_handler = signal(SIGUSR1, user_signal_handler);
  if(prev_handler == SIG_ERR){
    printf("handler failed %#x\n", prev_handler);
    return -1;
  }

  int fd = open(FILE_NAME, O_RDONLY);
  if(fd == -1){
    printf("open failed\n");
    return -1;
  }

  int offset = lseek(fd, sizeof(uint32_t), SEEK_SET);
  if(offset != sizeof(uint32_t)){
    printf("lseek failed\n");
    return -1;
  }

  while(1){
    for(uint32_t i = 0; i < period; ++i){
      if(i == 0){
        led(1);
      }else if(i == (period / 2)){
        led(0);
      }
      usleep(100);
      if(read_ready){
        int bytes_read = read(fd, &period, sizeof(uint32_t));
        if(bytes_read == -1){
          printf("read failed\n");
          return -1;
        }
        printf("%s: updated period to %d\n", argv[0], period);
        offset = lseek(fd, sizeof(uint32_t), SEEK_SET);
        if(offset != sizeof(uint32_t)){
          printf("lseek failed\n");
          return -1;
        }
        read_ready = 0;
        break;
      }
    }
  }
  return 0;
}
