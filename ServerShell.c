#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

#define PORT 4444
#define BUFFER_SIZE 1024

int main(){
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);;

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_port = htons(PORT);

    bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    listen(sockfd, 5);
    int client_sock = accept(sockfd, (struct sockaddr *)&client_addr, &addr_len);
    if (client_sock < 0) {
        perror("accept");
        close(sockfd);
        return 1;
    }
    char buffer[BUFFER_SIZE];
    int bytes_read = recv(client_sock, buffer, sizeof(buffer), 0);
    if (bytes_read < 0) {
        perror("recv");
        close(client_sock);
        close(sockfd);
        return 1;
    }
    close(sockfd);
    close(client_sock);
    return 0;
}