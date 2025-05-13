#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/io.h>
#include <termios.h>
#include <pthread.h>

_Atomic int counter = 1;

void *thread(void *str){
    while(counter){
        printf("%s", (char *)str);
        fflush(stdout);
        sleep(1);
    }
    return NULL;
}

void set_raw_mode(struct termios *status){
    struct termios new;
    tcgetattr(STDIN_FILENO, status);
    new = *status;
    new.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new);
}

void reset_mode(struct termios *status){
    tcsetattr(STDIN_FILENO, TCSANOW, status);
}

typedef struct {
    void (*SetRawTTY)(struct termios *status);
    void (*ResetMode)(struct termios *status);
} Terminal;

static Terminal terminal = {set_raw_mode, reset_mode};

int main(){
    pthread_t tid;
    struct termios old;
    char *str = "Hello world\n";
    pthread_create(&tid, NULL, thread, (void *)str);
    terminal.SetRawTTY(&old);
    char c;
    while(1){
        read(STDIN_FILENO, &c, 1);
        if(c == 'q'){
            counter = 0;
            break;
        }
    }
    pthread_join(tid, NULL);
    terminal.ResetMode(&old);
    return 0;
}