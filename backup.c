#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <net/if.h>

#define MAX_LINE_LENGTH 1024 * 4

typedef struct ifreq ifreq;
typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr sockaddr;

typedef struct {
  char *(*GetNameIface)(int,const char *);
  void *(*ConnectServer)(int,int,char *,int,const char *);
  void *(*ServerClient)(int,int,char *,int);
} Network;

char *get_ip_addr(int fd,const char *iface){
  ifreq ifr;
  strncpy(ifr.ifr_name, iface, IFNAMSIZ - 1);
  if(ioctl(fd, SIOCGIFADDR, &ifr) < 0){
    perror("ioctl");
    return NULL;
  }
  sockaddr_in *ip_addr = (sockaddr_in *)&ifr.ifr_addr;
  return inet_ntoa(ip_addr->sin_addr);
}

void *connect_to_server(int sockfd,int family,char *ip_addr,int port,const char *send_data){
  sockaddr_in server_addr;
  server_addr.sin_family = family;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = inet_addr(ip_addr);
  memset(server_addr.sin_zero,0,sizeof(server_addr.sin_zero));

  if(connect(sockfd,(sockaddr *)&server_addr,sizeof(server_addr)) < 0){
    perror("connect");
    return NULL;
  }
  send(sockfd,send_data,strlen(send_data),0);
  char *buffer = malloc(MAX_LINE_LENGTH);
  recv(sockfd,buffer,MAX_LINE_LENGTH,0);
  return buffer;
}

void *server_client(int sockfd,int family,char *ip_addr,int port){
    sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    
    server_addr.sin_family = family;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip_addr);
    memset(server_addr.sin_zero,0,sizeof(server_addr.sin_zero));

    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

    if(bind(sockfd,(sockaddr *)&server_addr,sizeof(server_addr)) < 0){
        perror("bind");
        return NULL;
    }
    if(listen(sockfd, 5) < 0){
        perror("listen");
        return NULL;
    }
    int client_sockfd = accept(sockfd, (sockaddr *)&client_addr, &addr_len);
    if(client_sockfd < 0){
        perror("accept");
        return NULL;
    }
    char *buffer = malloc(MAX_LINE_LENGTH);
    recv(client_sockfd, buffer, MAX_LINE_LENGTH, 0);
    return buffer;
}

static Network Net = {get_ip_addr,connect_to_server,server_client};

int main(){
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    char *buff = malloc(sizeof(char) * IFNAMSIZ);
    strncpy(buff, Net.GetNameIface(sockfd, "wlan0"), IFNAMSIZ);
    printf("IP Address: %s\n", buff);
    free(buff);
    return 0;
}
