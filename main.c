#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

typedef struct {
    void (*Println)(const char *)
} Standard;


void print_char(const char *str){
    write(STDOUT_FILENO, str, strlen(str));
}

static Standard std = {print_char};

int main(){
    std.Println("Hello world");
    return 0;
}