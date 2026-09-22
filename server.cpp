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
#include <errno.h>
#include <cassert>

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

// rbuf = read buffer
// wbuf = write buffer
// the server recieves first, and client sends first,
// then the server sends, and client recieves
static int32_t one_request(int connfd){
    // 4 bytes header
    char rbuf[4 + k_max_msg];
    errno = 0;
    // read message length (4 bytes, int 32)
    int32_t err = read_full(connfd, rbuf, 4);
    if (err) {
        msg(errno == 0 ? "EOF" : "read() error");
        return err;
    }

    uint32_t len = 0;
    // copy message length into len
    memcpy(&len, rbuf, 4);
    // printf("%u\n",len);
    if (len > k_max_msg) {
        msg("too long");
        return -1;
    }

    // request body
    // read message after byte 4
    err = read_full(connfd, &rbuf[4], len);
    if (err) {
        msg("read() error");
        return err;
    }

    printf("client sends (%u bytes): %.*s\n", len, len, &rbuf[4]);

    // reply back to client
    const char reply[] = "world";
    char wbuf[4 + sizeof(reply)];
    len = (uint32_t)strlen(reply);
    memcpy(wbuf, &len, 4);
    memcpy(&wbuf[4], reply, len);
    return write_all(connfd, wbuf, 4 + len);
}

    // struct addrinfo {
    //            int              ai_flags;
    //            int              ai_family;
    //            int              ai_socktype;
    //            int              ai_protocol;
    //            socklen_t        ai_addrlen;
    //            struct sockaddr *ai_addr;
    //            char            *ai_canonname;
    //            struct addrinfo *ai_next;
    // };

int main()
{
    struct addrinfo hints;
    struct addrinfo *res; // linked list of results
    int status;
    memset(&hints, 0, sizeof hints); // make sure the struct is empty
    hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
    hints.ai_flags = AI_PASSIVE;     // wildcard IP 0.0.0.0
    
    // this returns TCP ip address structs for local wildcard interfaces (0.0.0.0)
    // used by the server to bind and accept incoming connections on all its interfaces
    status = getaddrinfo(NULL, "1234", &hints, &res);
    if (status != 0) {
        die("getaddrinfo");
    }
    // protocol is 0 because we zeroed all of hints
    int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd < 0) {
        freeaddrinfo(res);
        die("socket");
    }

    int val = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
    
    // old way
    // struct sockaddr_in addr = {};
    // addr.sin_family = AF_INET;
    // addr.sin_port = htons(1234);        // port
    // addr.sin_addr.s_addr = htonl(0);    // wildcard IP 0.0.0.0
        
    // binds the socket descriptor to a local IP address and port (e.g., 0.0.0.0:1234)
    // int bind(int sockfd, struct sockaddr *my_addr, int addrlen);
    int rv = bind(fd, res->ai_addr, res->ai_addrlen);
    // error handler
    if (rv) { freeaddrinfo(res); die("bind()"); }

    // SOMAXCONN is the maximum number of pending TCP connects that can wait in a socket's listen queue 
    // before the OS starts dropping new connection requests.
    // listen for connections to server
    rv = listen(fd, SOMAXCONN);
    if (rv) { die("listen()"); }
    // free here because code after while (true) is unreachable because there's no breaking condition
    freeaddrinfo(res);

    while (true) {
        // can also use sockaddr_storage
        struct sockaddr_in client_addr = {};
        socklen_t addrlen = sizeof(client_addr);
        int connfd = accept(fd, (struct sockaddr *)&client_addr, &addrlen);
        if (connfd < 0) {
            continue;   // error
        }

        // do_something(connfd);

        // only serves one client connection at once
        while (true) {
            int32_t err = one_request(connfd);
            if (err) {
                break;
            }
        }

        close(connfd);
    }    
};
