#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>

int main(int argc,char **argv){
    if(argc < 2){
        printf("Usage: %s <interface>\n", argv[0]);
        printf("Example: %s eth0\n", argv[0]);
        printf("Example: %s wlan0\n", argv[0]);
        return 1;
    }
    char filter[50];
    strncpy(filter, argv[1], sizeof(filter) - 1);
    struct ifreq ifr;
    int sockfd = socket(AF_INET,SOCK_DGRAM,0);;
    strncpy(ifr.ifr_name, filter, IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';
    ioctl(sockfd, SIOCGIFADDR, &ifr);
    struct sockaddr_in *ip_addr = (struct sockaddr_in *)&ifr.ifr_addr;
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &ip_addr->sin_addr, ip, sizeof(ip));
    printf("IP Address: %s\n", ip);
    close(sockfd);
    return 0;
}