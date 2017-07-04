#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <netinet/in.h>
#include <errno.h>
#include <iostream>
#include <netdb.h>

int main() {
    int sock = socket(AF_INET, SOCK_DGRAM,0);
    struct sockaddr_in myaddr;
    memset((char *) &myaddr, 0, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myaddr.sin_port = htons(1235);
    if(bind(sock, (struct sockaddr *)&myaddr,sizeof(myaddr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    struct hostent *hp;
    hp = gethostbyname("localhost");
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(1234);
    memcpy((void *)&servaddr.sin_addr,"127.0.0.1", 9);
    std::string msg = "TEST";
    if (sendto(sock, msg, 4, 0, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("sendto failed");
        exit(EXIT_FAILURE);
    }

}
