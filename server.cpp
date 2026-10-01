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

#include "Socket.hpp"

using namespace std;

int main()
{
    int fd = -1;
    struct addrinfo hints{};
    RedClone::Socket sock(fd, hints, "127.0.0.1", "1234");

    // Allow binding to the port even if it is in TIME_WAIT
    int val = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0)
    {
        die("setsockopt failure");
    }

    // bind fd to socket
    if (bind(fd, sock.getRes()->ai_addr, sock.getRes()->ai_addrlen))
    {
        die("Bind failure");
    }

    if (listen(fd, SOMAXCONN))
    {
        die("Listen failure");
    }

    while (true)
    {
        int acceptFd = accept(fd, nullptr, nullptr);

        if (acceptFd < 0)
        {
            die("Accept failure");
            break;
        }

        while (true)
        {
            sock.readMsg(acceptFd);
            sock.writeMsg(acceptFd);
        }
    }
};
