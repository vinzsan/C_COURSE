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

// Assembly Backup

#define NULL ((void *)0)
#define PROT_WRITE 0x02
#define MAP_ANONYMOUS 0x20
#define MAP_PRIVATE 0x02

static inline long print(char *str){
  volatile register long buffer = 0;
  while(buffer[str]) buffer++;
  __asm__ __volatile__(
    "movq $1,%%rax\n\t"
    "movq $1,%%rdi\n\t"
    "movq %0,%%rsi\n\t"
    "movq %1,%%rdx\n\t"
    "syscall"
    :
    :"r"(str),"r"(buffer)
    :"%rax","%rdi","%rsi","%rdx"
  );
  return buffer;
}

static void *mmap(void *position,long long size,int PROT,int MAP,int fd,int flags){
  void *ret = 0;
  __asm__ __volatile__(
    "movq $9,%%rax\n\t"
    "movq %1,%%rdi\n\t"
    "movq %2,%%rsi\n\t"
    "movq %3,%%rdx\n\t"
    "movq %4,%%r10\n\t"
    "movq %5,%%r8\n\t"
    "movq %6,%%r9\n\t"
    "syscall\n\t"
    "movq %%rax,%0"
    : "=r"(ret)
    :"r"(position),"r"(size),"r"((long)PROT),"r"((long)MAP),"r"((long)fd),"r"((long)flags)
    : "%rax","%rdi","%rsi","%rdx","%r10","%r8","%r9"
  );
  return ret;
}

static volatile void munmap(void *pos,long long size){
  __asm__ __volatile__(
    "movq $11,%%rax\n\t"
    "movq %0,%%rdi\n\t"
    "movq %1,%%rsi\n\t"
    "syscall"
    :
    :"r"(pos),"r"((long)size)
    :"%rax","%rdi","%rsi"
  );
}

static void *strcopy(char *dest, char *str) {
  if (!dest || !str) return NULL; 
  char *d = dest;
  while ((*d++ = *str++));
  return dest;
}

typedef struct{
  void *(*VirtualAlloc)(void *position,long long size,int PROT,int MAP,int fd,int flags);
} standard;

static standard Win64 = {mmap};

int main(){
  char *buffer = mmap(NULL, 1024, PROT_WRITE,MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  strcopy(buffer, "Hello world");
  print("Hello world\n");
  print(buffer);
  munmap(buffer, 1024);
  return 0;
}
