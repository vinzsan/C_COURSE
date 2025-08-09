#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstdlib>
#include <unistd.h>
#include <poll.h>
#include <cstring>
#include <future>
#include <termios.h>
#include <cstdlib>
#include <cstdio>
#include <vector>
//#include <promises>

#define PORT 8080


struct Termios{
    termios old;
    Termios(){
        tcgetattr(STDIN_FILENO,&old);
        termios tty = old;
        tty.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO,TCSANOW,&tty);
    }
    ~Termios(){
        tcsetattr(STDIN_FILENO,TCSANOW,&old);
    }
};

void multi_thread(int &sockfd,std::promise<ssize_t> result){
    char string[128];
    //const char* header = "HTTP/1.1 200 OK\r\nContent-type:text/html\r\nContent-length:32\r\n\r\n<h1>Hello world</h1>";
    std::snprintf(string,sizeof(string),"HTTP/1.1 200 OK\r\nContent-type:text/html\r\nContent-length:32\r\n\r\n%s","<h1>Hello world</h1>");
    send(sockfd,string,std::strlen(string),0);
    char *buff = new char[1024];
    if(buff == NULL){
        std::cerr << "Error allocate buffer" << std::endl;
        return;
    }
    ssize_t max = recv(sockfd,buff,1024,0);
    std::cout << buff << std::endl;
    //std::free(buff);
    delete[] buff;
    close(sockfd);
    result.set_value(max);
}

int main(int argc,char **argv){
    if(argc < 2){
        std::cout << "Use " << argv[0] << " <port>" << std::endl;
        return -1;
    }
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(sockfd < 0){
        std::cerr << "Error made socket" << std::endl;
        return -1;
    }

    int setopt = 1;
    if(setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&setopt,sizeof(setopt)) < 0){
        std::cerr << "Error set socket option" << std::endl;
        close(sockfd);
        return -1;
    }
    
    //termios old,tty;
    short port = static_cast<short>(std::stoi(argv[1]));
    Termios tg;
    try{
        sockaddr_in address;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_family = AF_INET;
        address.sin_port = htons(port);
        std::memset(&address.sin_zero,0,sizeof(address.sin_zero));
        socklen_t len = sizeof(address);
        if(bind(sockfd,(sockaddr *)&address,len) || listen(sockfd,SOMAXCONN) < 0){
            throw std::runtime_error("Error bind");
        }
        //tcgetattr(STDIN_FILENO,&old);
        //tty = old;
        //tty.c_lflag &= ~(ICANON | ECHO);
        //tcsetattr(STDIN_FILENO,TCSANOW,&tty);
        pollfd fds[2];
        fds[0].events = POLLIN;
        fds[0].fd = sockfd;
        fds[1].fd = STDIN_FILENO;
        fds[1].events = POLLIN;
        while(1){
            int ret = poll(fds,2,5000);
            std::vector<std::future<ssize_t>> vec;
            std::promise<ssize_t> prom;
            vec.push_back(prom.get_future());
            //std::future<ssize_t> result = prom.get_future();

            if(ret < 0){
                //std::cerr << "Error poll" << std::endl;
                throw std::runtime_error("Error pollin file descriptor");
            }
            if(fds[0].revents & POLLIN){
                int clientfd = accept(sockfd,(sockaddr *)&address,&len);
                if(clientfd < 0){
                    std::cout << "Client Disconnect" << std::endl;
                    continue;
                }
                std::thread(multi_thread,std::ref(clientfd),std::move(prom)).detach();
                //std::cout << "Size result : " << result.get() << std::endl;
                //tid.detach();
            }
            if(fds[1].revents & POLLIN){
                char c = getchar();
                if(c == 'q'){
                    std::cout << "Exiting server" << std::endl;
                    break;
                }
            }
            for(std::vector<std::future<ssize_t>>::iterator it = vec.begin();it != vec.end();){
                if(it->wait_for(std::chrono::seconds(0)) == std::future_status::ready){
                    ssize_t result = it->get();
                    std::cout << "Size : " << result << std::endl;
                    it = vec.erase(it);
                }
                else{
                    ++it;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    catch(std::exception &e){
        //tcsetattr(STDIN_FILENO,TCSANOW,&old);
        //if(tcsetattr(STDIN_FILENO,TCSANOW,&old) != 0){
        //    std::cerr << "Error termios" << std::endl;
        //}
        std::cout << "Exception catch : " << e.what() << std::endl;
    }
    //tcsetattr(STDIN_FILENO,TCSANOW,&old);
    close(sockfd);
    return 0;
}
