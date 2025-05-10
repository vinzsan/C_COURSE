#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define PORT 80
#define IP_ADDR "192.168.100.167" // This is my Laptop ip,u can change it

int main(){
  struct sockaddr_in server_addr;
  socklen_t addr_len = sizeof(server_addr);

  int sockfd = socket(AF_INET,SOCK_STREAM,0);
  if(sockfd < 0){
    perror("Socket");
    return 1;
  }

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);
  server_addr.sin_addr.s_addr = inet_addr(IP_ADDR);
  memset(server_addr.sin_zero,0,sizeof(server_addr.sin_zero));

  if((connect(sockfd,(struct sockaddr *)&server_addr,addr_len)) < 0){
    perror("Connect");
    close(sockfd);
    return 1;
  }

  char *header = "Hello from Client";

  send(sockfd,header,strlen(header),0);

  char *buffer = (char *)malloc(1024);

  size_t len = recv(sockfd,buffer,1024,0);
  write(1,buffer,len);

  free(buffer);
  close(sockfd);
  return 0;
}
