#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdatomic.h>

pthread_t tid;
atomic_int counter = 1;
atomic_int sockfd;

void handler(int sig){
    printf("Handler caught %d\n", sig);
    close(sockfd);
    counter = 0;
}

void *thread_handler(void *args){
    int client_sock = *(int *)args;
    free(args);
    char buffer[1024];

    char *response = "Hello This is Server Local\n";
    write(client_sock, response, strlen(response));
    
    while(atomic_load(&counter)){
        size_t len = read(client_sock, buffer, 1024);
        if(len <= 0) break;
        printf("[client] : ");
        fflush(stdout);
        write(STDOUT_FILENO, buffer, len);
        sleep(1);
    }
    close(client_sock);
    return NULL;
}

int main(){
    struct sockaddr_in address;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
        perror("Eror made socket");
        return 1;
    }

    address.sin_family = AF_INET;
    address.sin_port = htons(80);
    address.sin_addr.s_addr = INADDR_ANY;
    memset(&address.sin_zero, 0, sizeof(address.sin_zero));

    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

    if((bind(sockfd,(struct sockaddr *)&address,sizeof(address))) < 0){
        perror("Error bind");
        shutdown(sockfd, SHUT_RDWR);
        return 1;
    }
    printf("[INFO] : Server run at port 80\n");
    printf("[INFO] : INADDR_SET %d\n", INADDR_ANY);
    printf("[INFO] : Press Ctrl + C to close\n");

    listen(sockfd, 1);
    struct sigaction sig;
    sig.sa_flags = SA_RESTART;
    sig.sa_handler = handler;
    sigemptyset(&sig.sa_mask);
    sigaction(SIGINT, &sig, NULL);

    while(atomic_load(&counter)){
        socklen_t addrlen = sizeof(address);
        int *client_fd = malloc(sizeof(int));
        *client_fd = accept(sockfd, (struct sockaddr *)&address, &addrlen);

        pthread_t tid;
        pthread_create(&tid, NULL, thread_handler, client_fd);
        sleep(1);
        pthread_detach(tid);
    }
    close(sockfd);
    return 0;
}
