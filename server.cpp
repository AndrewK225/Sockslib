#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <netinet/in.h>
#include <errno.h>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>

int main() {
    /* Create a socket to listen on and bind it to port 1234*/
    struct sockaddr_in serv_addr, serv_addr2;
    struct epoll_event ev, events[10];
    int serv_sock = socket(AF_INET, SOCK_DGRAM,0);
    int serv_sock2 = socket(AF_INET,SOCK_DGRAM,0);
    int conn_sock, nfds, epollfd;
    if(serv_sock < 0) {
        perror("Socket Creation Error");
        exit(1);
    }
    memset((char*)&serv_addr,0,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(1234); 
    int set_option_on = 1;
    int res = setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, (char*) &set_option_on, 
    sizeof(set_option_on));  
    fcntl(serv_sock, F_SETFL, O_NONBLOCK); 
    if(bind(serv_sock,(struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Error binding");
        exit(1);
    }




    unsigned char buf[100];


    epollfd = epoll_create1(0);
    if (epollfd == -1) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }   

    ev.events = EPOLLIN;
    ev.data.fd = serv_sock;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, serv_sock, &ev) == -1) {
        perror("epoll_ctl: serv_sock");
        exit(EXIT_FAILURE);
    }
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, serv_sock2, &ev) == -1) {
        perror("epoll_ctl: serv_sock");
        exit(EXIT_FAILURE);
    }

    for (;;) {
/*
        int recvlen = recvfrom(serv_sock,buf,100,0,(struct sockaddr *)&serv_addr,(socklen_t*)sizeof(serv_addr));
        if (recvlen > 0) {
            buf[recvlen] = 0;
            printf("received message: \"%s\"\n", buf);
        }
*/
        std::cout << "in inf loop" << std::endl;        
        nfds = epoll_wait(epollfd, events, 10, -1);
        if (nfds == -1) {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }
        else {
            std::cout << nfds << std::endl;
        }
        //write(serv_sock,"TESTING\n",8);
        socklen_t addrlen = sizeof(serv_addr);
        for (int n = 0; n < nfds; ++n) {
            std::cout << "HERE" << std::endl;
            if (events[n].data.fd == serv_sock) {
                conn_sock = recvfrom(serv_sock,buf,100,0, (struct sockaddr *) &serv_addr,&addrlen); 

                if (conn_sock == -1) {
                    perror("recvfrom");
                    exit(EXIT_FAILURE);
                }
                //setnonblocking(conn_sock);
                write(1,buf,conn_sock);
            } else {
                std::cout << events[n].data.fd << std::endl;    
            }   
        }
    
    }
    
}   
