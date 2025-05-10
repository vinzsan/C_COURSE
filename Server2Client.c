#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define PORT_SERVER 80
#define IP_SERVER_LOCAL INADDR_ANY

int main(){
  struct sockaddr_in server_addr,client_addr;
  socklen_t addr_len = sizeof(client_addr);

  int sockfd = socket(AF_INET,SOCK_STREAM,0);
  if(sockfd < 0){
    perror("Socket");
    return 1;
  }

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT_SERVER);
  server_addr.sin_addr.s_addr = IP_SERVER_LOCAL;
  memset(server_addr.sin_zero,0,sizeof(server_addr.sin_zero));

  setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&(int){1},sizeof(int));

  bind(sockfd,(struct sockaddr *)&server_addr,sizeof(server_addr));
  listen(sockfd,1);

  int clientfd = accept(sockfd,(struct sockaddr *)&client_addr,&addr_len);
  if(clientfd < 0){
    perror("Accept");
    close(sockfd);
    return 1;
  }

  char *header = "Hello this is server";
  send(clientfd,header,strlen(header),0);
  char *buffer = (char *)malloc(1024);
  size_t len = recv(clientfd,buffer,1024,0);
  write(1,buffer,len);
  free(buffer);
  close(sockfd);
  return 0;
}
