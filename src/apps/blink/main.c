#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define FILE_NAME "blink.io"

int main(char *const argv[]){  
  if(argv[1] == NULL){
    printf("blink no args provided\n");
    return -1;
  }
  int fd = open(FILE_NAME, O_CREAT | O_RDWR);
  int file_size = lseek(fd, 0, SEEK_END);
  if(file_size == -1){
    printf("lseek failed\n");
    return -1;
  }else if(file_size == 0){
    uint32_t child_pid = fork(); 
    if(child_pid == -1){
      printf("fork failed\n");
      return -1;
    }else if(child_pid == 0){
      char *args[] = {"blinkd", argv[1], NULL};
      execv(args[0], args);
      return -1;
    }  
    printf("%s: created process %d\n", argv[0], child_pid);
    int bytes_written = write(fd, &child_pid, sizeof(uint32_t));
    if(bytes_written != sizeof(uint32_t)){
      printf("write failed %d\n", bytes_written);
      return -1;
    }
    close(fd);
    return 0;
  }

  int offset = lseek(fd, 0, SEEK_SET);
  if(offset == -1){
    printf("lseek offset: %d\n", offset);
    return -1;
  }

  uint32_t child_pid = -1;
  int bytes_read = read(fd, &child_pid, sizeof(uint32_t));
  if(bytes_read == -1){
    printf("read failed\n");
    return -1;
  }

  uint32_t period = atoi(argv[1]);
  int bytes_written = write(fd, &period, sizeof(uint32_t));
  printf("%s: sending update to blinkd\n", argv[0]);
  int ret = kill(child_pid, SIGUSR1);
  if(ret == -1){
    printf("kill failed: %d\n", ret);
    return -1;
  }
  close(fd);
  return 0;
}
