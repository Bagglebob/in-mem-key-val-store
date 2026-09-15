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
#include "common.h"

using namespace std;


static void do_something(int connfd) {
    char rbuf[64] = {};
    // change to recv once you know what to do with the flags
    ssize_t n = read(connfd, rbuf, sizeof(rbuf) - 1);
    if (n < 0) {
        msg("[SERVER] read() error");
        return;
    }
    printf("[SERVER] client says: %s\n", rbuf);

    char wbuf[] = "world";
    // change to send once you know what to do with the flags
    write(connfd, wbuf, strlen(wbuf));
}


int main()
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    int val = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    struct sockaddr_in addr = {};

    addr.sin_family = AF_INET;
    addr.sin_port = htons(1234);        // port
    addr.sin_addr.s_addr = htonl(0);    // wildcard IP 0.0.0.0
    
    int rv = bind(fd, (const struct sockaddr *)&addr, sizeof(addr));
    // error handler
    if (rv) { die("bind()"); }
    // SOMAXCONN (and the Linux kernel parameter net.core.somaxconn) is the maximum number of pending TCP
    // connections that can wait in a socket's listen queue before the operating system starts 
    // dropping new connection requests.
    rv = listen(fd, SOMAXCONN);
    if (rv) { die("listen()"); }

    // actually listen for connections to server
    while (true) {
        // accept
        struct sockaddr_in client_addr = {};
        socklen_t addrlen = sizeof(client_addr);
        int connfd = accept(fd, (struct sockaddr *)&client_addr, &addrlen);
        if (connfd < 0) {
            continue;   // error
        }
        // function above
        do_something(connfd);
        close(connfd);
    }
};
