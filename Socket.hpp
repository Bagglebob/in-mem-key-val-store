#ifndef SOCKET_H
#define SOCKET_H
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

#include "resp.h"
#include "common.h"
using namespace std;

namespace RedClone
{
    class Socket
    {
        struct addrinfo hints{};
        struct addrinfo *res{};

    public:
        // res is a pointer, so it expects a memory address
        Socket(int &fd, const struct addrinfo &hints, const char *ip, const char *port);

        const addrinfo *getRes() const;

        ssize_t readMsg(int &fd) const;

        ssize_t readRaw(int fd, std::string &in_buffer);

        ssize_t writeMsg(int &fd) const;

        ssize_t writeRaw(int fd, const std::string &data);
    };
}
#endif