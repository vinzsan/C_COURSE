#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int printchar(const char *str){
    size_t buffer = 0;
    while(buffer[str] != '\0'){
        buffer++;
    }
    size_t buffer_ret;
    asm(
        "movq $1,%%rax\n\t"
        "movq $1,%%rdi\n\t"
        "movq %1,%%rsi\n\t"
        "movq %2,%%rdx\n\t"
        "syscall\n\t"
        "movq %%rax, %0\n\t"
        : "=r"(buffer_ret)
        : "r"(str), "r"((long)buffer)
        : "%rax", "%rdi", "%rsi", "%rdx");
    return buffer_ret;
}

int main(){
    char *str = "Hello, World!\n";
    size_t length = printchar(str);
    printf("%d", length);
    return 0;
}