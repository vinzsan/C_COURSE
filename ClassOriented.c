#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#define STDOUT 1

void print_no_enter(const char *str){
  write(STDOUT,str,strlen(str));
}

struct standard1 {
  void (*Print)(const char *);
};

int main(){
  static struct standard1 std1 = {print_no_enter};
  std1.Print("Hello world\n");

  struct standard1 *std2 = malloc(sizeof(struct standard1)); // Create like New in C++
  if(std2 == NULL){
    perror("Memory");
    return 1;
  }
  std2->Print = print_no_enter;
  std2->Print("Hello world with New Keyword like in C++\n");
  free(std2); // like Delete keyword in C++
  return 0;
}
