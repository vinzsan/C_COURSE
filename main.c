#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

typedef struct {
    void (*Println)(const char *);
} Standard;

void print_char(const char *str){
    write(STDOUT_FILENO, str, strlen(str));
}

int main(){
    Standard *std = malloc(sizeof(Standard));
    std->Println = print_char;
    std->Println("Hello world");
    free(std);
    return 0;
}