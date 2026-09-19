// --- Universal C++ Network Boilerplate ---
#include <iostream>
#include <string>
#include <vector>
#include <cstring>      // memset, strlen
#include <cstdio>       // perror
#include <cstdlib>      // exit
#include <unistd.h>     // read, write, close
#include <sys/socket.h> // socket, bind, listen, accept, setsockopt
#include <netinet/in.h> // sockaddr_in, htons, htonl, AF_INET
#include <arpa/inet.h>  // inet_ntoa, inet_pton
#include <netdb.h>      // Required for getaddrinfo, freeaddrinfo, struct addrinfo
#include "common.h"

using namespace std;

int main(){
    struct addrinfo hints;
    struct addrinfo *res; // linked list of results
    int status;
    memset(&hints, 0, sizeof hints); // make sure the struct is empty
    hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets

    status = getaddrinfo("127.0.0.1", "1234", &hints, &res);
    if (status != 0) {
        die("getaddrinfo");
    }


    int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd < 0) {
        freeaddrinfo(res);
        die("socket()");
    }
    // old way
    // struct sockaddr_in addr = {};
    // addr.sin_family = AF_INET;
    // addr.sin_port = ntohs(1234);
    // addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK);  // 127.0.0.1

    int rv = connect(fd, res->ai_addr, res->ai_addrlen);
    if (rv < 0) {
        freeaddrinfo(res);
        die("connect");
    }
    freeaddrinfo(res);

    char msg[] = "hello";
    write(fd, msg, strlen(msg));

    char rbuf[64] = {};
    ssize_t n = read(fd, rbuf, sizeof(rbuf) - 1);
    if (n < 0) {
        die("read");
    }
    printf("[CLIENT] server says: %s\n", rbuf);
    close(fd);
};
