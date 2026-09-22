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
// rbuf = read buffer
// wbuf = write buffer
// the server recieves first, and client sends first,
// then the server sends, and client recieves
static int32_t query(int fd, const char *text){
    // get message length (32 bit)
    uint32_t len = (uint32_t)strlen(text);

    if (len > k_max_msg) {
        return -1;
    }
    // create buffer to hold message + length (32/8 = 4 bytes, a 32 bit int is 4 bytes)
    char wbuf[4 + k_max_msg];

    // copy len to the first 4 bytes in wbuf
    memcpy(wbuf, &len, 4);

    // copy text into wbuf starting from after byte/index 4
    memcpy(&wbuf[4], text, len);
    
    // since write_all returns -1 on failure, non-zero values evaluate to true entering the if block
    if (int32_t err = write_all(fd, wbuf, 4 + len)) {
        return err;
    }
    // client receives
    // 4 byte length + msg
    char rbuf[4 + k_max_msg];
    errno = 0;
    int32_t err = read_full(fd, rbuf, 4);
    if (err) {
        msg(errno == 0 ? "EOF" : "read() error");
        return err;
    }
    // grab length of received message
    memcpy(&len, rbuf, 4);
    if (len > k_max_msg) {
        msg("too long");
        return -1;
    }

    // read server's message
    err = read_full(fd, &rbuf[4], len);
    if (err) {
        msg("read() error");
        return err;
    }
    printf("server says: %.*s\n", len, &rbuf[4]);
    return 0;
}

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
   
    int32_t err = query(fd, "hello1");
    if (err) {
        goto L_DONE;
    }
    err = query(fd, "hello2");
    if (err) {
        goto L_DONE;
    }
L_DONE:
    close(fd);
    return 0;

};
