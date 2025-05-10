#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(){
  char *buffer = (char *)malloc(1024 * sizeof(char));
  char *buffer2 = (char *)calloc(1024 * sizeof(char),sizeof(char));
  char *buffer3 = (char *)realloc(buffer2,1024 * sizeof(char));
  if(buffer == NULL){
    perror("Error Allocation Memory");
    return 1;
  }

  fgets(buffer,1024 * sizeof(char),stdin);
  printf("%s",buffer);

  char *str = "Hello world";

  strncpy(buffer2,str,1024 * sizeof(char));
  printf("%s",buffer2);

  strncpy(buffer3,str,1024 * sizeof(char));
  printf("%s",buffer3);

  free(buffer);
  free(buffer3);
  return 0;
}
