#include <sys/epoll.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <errno.h>
#include <string>
#include <cstring>
#include <vector>
#include <unordered_map>
#include <stdio.h>

typedef struct _params {
    int local_port, dest_port;
    std::string dest_ip = "0";
    std::string local_ip = "0";
    std::string proto;
} Params;


int create_socket(Params *pm) {
    struct sockaddr_in addr;
    int sock,set_opt_on,res;

    /* setup the sockaddr struct */
    memset((char*) &addr,0,sizeof(addr));
    addr.sin_family = AF_INET;
    if(pm -> local_ip.compare("0") == 0) {
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    addr.sin_port = htons(pm -> local_port);

    /*if UDP socket */
    if(pm -> proto.compare("udp") == 0) {
        sock = socket(AF_INET, SOCK_DGRAM, 0);
        if(sock < 0) {
            perror("UDP Socket Creation Error");
            exit(1);
        }
        set_opt_on = 1;
        res = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
                   (char*) &set_opt_on, sizeof(set_opt_on));
        fcntl(sock, F_SETFL, O_NONBLOCK);
        if(bind(sock,(struct sockaddr *)&addr, sizeof(addr))< 0) {
            perror("Error Binding socket");
            exit(1);
        }
        return sock;
    }
} 
/*
    else if (proto.compare("tcp") == 0){
    sock = socket(AF_INET, SOCK_STREAM,0);
    if(sock < 0) {
        perror("TCP Socket Creation Error");
        exit(1);
    }
    set_opt_on = 1;
    res = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
                    (char*) &set_opt_on, sizeof(set_opt_on));
    fcntl(sock,F_SETFL, O_NONBLOCK);
    if(bind(sock,(struct sockaddr *) &addr, sizeof(addr)) < 0) {
        perror("Error Binding socket");
        exit(1);
    }
    
    
}
*/


/* In the next step, want a generic handle_event that will call other fcns.  This way, when tcp is implemented, will handle connection request as well as data*/


int use_polling(std::string method, int (*create_sock)(Params *pm),bool (*handle_event)(int)) {
    int data;
    unsigned char buf[100];
    /* Would need multiple of these structs for multiple sockets */
    Params pm1;
    pm1.proto = "udp";
    pm1.local_port = 1234;
    /* hashtable of socket fds */
    std::unordered_map<int,int> sockets;
    int sock_fd = create_sock(&pm1);
    sockets[sock_fd] = 1;

    if(method.compare("epoll") == 0) {
        bool cont = true;
        struct epoll_event ev, events[10];
        int epollfd = epoll_create1(0);
        if(epollfd < 0) {
            perror("Error with epoll_create1");
            exit(1);
        }
        /* Look for input events */
        ev.events = EPOLLIN; 
    
        for(auto it = sockets.begin(); it != sockets.end(); ++it) {
            ev.data.fd = it -> first;
            if(epoll_ctl(epollfd, EPOLL_CTL_ADD, it->first, &ev) == -1) {
                perror("epoll_ctl error");
            }
        }
        while(cont) {
            int nfds = epoll_wait(epollfd, events, 10 , -1);
            if(nfds == -1) {
                perror("epoll_wait");
                exit(1);
            }
            for(int n = 0; n < nfds; ++n) {
            
                if(sockets[events[n].data.fd]) {
                    cont = handle_event(events[n].data.fd);
                } else {
                    std::cout << events[n].events << std::endl;
                }
            }
        }
    }


}

/* In the future, want a generic event handler that would call this if appropriate */
bool handle_data(int fd) {
    unsigned char buf[100];
    
    int data = read(fd,buf,100);
    if(data < 0 ) {
        perror("error with read");
        exit(1);
    }
    write(1,buf,data);
    std::string str(buf,buf +sizeof(buf)/sizeof(buf[0]));
    if(str.compare("quit") == 0) {
        return false;
    }
    return true;


}
int main() {
    use_polling("epoll",&create_socket,&handle_data);

}
