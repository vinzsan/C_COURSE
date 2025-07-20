#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <poll.h>
#include <termios.h>
#include <string.h>
#include <pthread.h>

#define MAX_FD_SET 10

void *multi(void *args){
  int32_t clientfd = *(int32_t *)args;
  free(args);

  char *http = "HTTP/1.1 200 OK\r\nContent-type: text/html\r\nContent-length: 1024\r\n\r\n<h1>Hello world</h1>";
  
  send(clientfd,http,strlen(http),0);
  char *buffer = malloc(1024 * sizeof(char));
  if(buffer == NULL){
    return NULL;
  }
  size_t max = recv(clientfd,buffer,1024 * sizeof(char),0);
  write(STDOUT_FILENO,buffer,max);
  close(clientfd);
  free(buffer);
  return NULL;
}

int main(){
  struct sockaddr_in addr = {
    .sin_family = AF_INET,
    .sin_port = htons(8080),
    .sin_addr.s_addr = INADDR_ANY
  };
  memset(&addr.sin_zero,0,sizeof(addr.sin_zero));
  
  int sockfd = socket(AF_INET,SOCK_STREAM,0);
  if(sockfd < 0){
    perror("Error made socket");
    return -1;
  }
  
  setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&(int){1},sizeof(int));
  
  if(bind(sockfd,(struct sockaddr *)&addr,sizeof(addr)) || listen(sockfd,10) < 0){
    perror("Error bind");
    close(sockfd);
    return -1;
  }
  struct pollfd fds[MAX_FD_SET];
  nfds_t ndfs = 0;
  fds[0].fd = STDIN_FILENO;
  fds[0].events = POLLIN;
  ndfs++;
  fds[1].fd = sockfd;
  fds[1].events = POLLIN;
  ndfs++;
  struct termios old,new;
  tcgetattr(STDIN_FILENO,&old);
  new = old;
  new.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO,TCSANOW,&new);
  
  while(1){
    int ret = poll(fds,2,1000);
    for(int i = 0;i < ndfs;i++){
      if(fds[i].revents & POLLIN){
        if(fds[i].fd == sockfd){
          if(fds[i].revents & POLLIN){
            int32_t *clientfd = malloc(sizeof(int32_t));
            if(clientfd == NULL){
              perror("Error allocate");
              break;
            }
            socklen_t addlen = sizeof(addr);
            *clientfd = accept(sockfd,(struct sockaddr *)&addr,&addlen);
            if(*clientfd < 0){
              printf("Client disconnect\n");
              continue;
            }
            pthread_t tid;
            pthread_create(&tid,NULL,multi,clientfd);
            pthread_detach(tid);
            }
        }
      }
      if(fds[i].revents & POLLIN){
        if(fds[i].fd == STDIN_FILENO){
          char c = getchar();
          if(c == 'q'){
            printf("Exiting\n");
            goto end;
            break;
          }
        }
      }
    }
    usleep(10000);
  }
end:
  tcsetattr(STDIN_FILENO,TCSANOW,&old);
  close(sockfd);
  return 0;
}
