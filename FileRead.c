#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int main(int argc,char **argv){
  if(argc < 2){
    printf("Usage: %s <filename>\n", argv[0]);
    return 1;
  }
  char filter[50];
  strncpy(filter, argv[1], sizeof(filter) - 1);
  int file = open(filter,O_RDONLY);
  if(file < 0){
    perror("Error opening file");
    return 1;
  }
  size_t max = lseek(file, 0, SEEK_END);
  char *buffer = malloc(max * sizeof(char));
  if(buffer == NULL){
    perror("Error allocating memory");
    close(file);
    return 1;
  }
  lseek(file, 0, SEEK_SET);
  read(file, buffer, max);
  write(STDOUT_FILENO, buffer, max);
  free(buffer);
  close(file);
  return 0;
}
