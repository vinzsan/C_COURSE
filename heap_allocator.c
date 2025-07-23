//#define HEAP_SIZE 4096
#define NULL ((void *)0)
char *heap_base = NULL;
char *heap_end  = NULL;

typedef long intptr_t;

void *_brk(void *ptr){
    void *new;
    __asm__ __volatile__(
        "movq $12,%%rax\n\t"
        "movq %1,%%rdi\n\t"
        "syscall\n\t"
        "movq %%rax,%0"
        :"=r"(new)
        :"r"(ptr)
        :"rax","rdi","rsi"
    );
    return new;
}

void *my_sbrk(intptr_t increment) {
    static void *program_break = 0;

    if (program_break == 0) {
        program_break = (void *) _brk(0);
    }
    void *old_break = program_break;
    void *new_break = (void *)((char *)program_break + increment);
    if (_brk(new_break) == 0) {
        return (void *) -1;
    }
    program_break = new_break;
    return old_break; 
}

void *simple_malloc(int size) {
    if (!heap_base) {
        heap_base = my_sbrk(size); 
        heap_end  = heap_base;
    }

    void *ptr = heap_end;
    heap_end += size;                
    return ptr;                      
}

char *strcpy(char *dest,char *src){
    if(!dest || !src){
        return NULL;
    }
    char *d = dest;
    while((*d++ = *src++));
    return dest;
}

int _start(){
    char *ptr = simple_malloc(1024);
    strcpy(ptr,"Hello world\n");
    //free(ptr);
    __asm__ __volatile__("int3");
}
