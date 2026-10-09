// --- Universal C++ Network Boilerplate ---
#include <iostream>
#include <string>
#include <vector>
#include <cstring>      // memset, strlen
#include <cstdio>       // perror
#include <cstdlib>      // exit
#include <unistd.h>     // read, write, close
#include <sys/socket.h> // socket, bind, listen, accept, setsockopt
#include <sys/select.h> // non blocking I/O
#include <netinet/in.h> // sockaddr_in, htons, htonl, AF_INET
#include <arpa/inet.h>  // inet_ntoa, inet_pton
#include <netdb.h>      // Required for getaddrinfo, freeaddrinfo, struct addrinfo
#include <errno.h>
#include <cassert>

#include "Socket.hpp"
#include "handler.h"

using namespace std;

int main()
{
    int fd_listen = -1;
    struct addrinfo hints{};
    RedClone::Socket sock(fd_listen, hints, "127.0.0.1", "1234");

    // Allow binding to the port even if it is in TIME_WAIT
    int val = 1;
    if (setsockopt(fd_listen, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0)
    {
        die("setsockopt failure");
    }

    // bind fd to socket
    if (bind(fd_listen, sock.getRes()->ai_addr, sock.getRes()->ai_addrlen))
    {
        die("Bind failure");
    }

    if (listen(fd_listen, SOMAXCONN))
    {
        die("Listen failure");
    }

    // non blocking i/o
    fd_set master_set;
    FD_ZERO(&master_set);
    FD_SET(fd_listen, &master_set);
    int max_socket = fd_listen;

    RedClone::KVStore kv_store{};
    // each fd/socket has it's own buffer
    std::unordered_map<int, std::string> in_buffer{};
    while (true)
    {
        // non blocking i/o
        fd_set reads; // creates a copy so the original isn't destroyed to fuck
        reads = master_set;
        if (select(max_socket + 1, &reads, 0, 0, 0) < 0)
        {
            fprintf(stderr, "select() failed. (%d)\n", errno);
            return 1;
        }
        // int acceptFd = -1;
        for (int i = 1; i <= max_socket; ++i)
        {
            if (FD_ISSET(i, &reads))
            {
                // if it's a listening socket then accept, otherwise read
                if (i == fd_listen)
                {
                    int acceptFd = accept(fd_listen, nullptr, nullptr);
                    FD_SET(acceptFd, &master_set);
                    if (acceptFd > max_socket)
                    {
                        max_socket = acceptFd;
                    }
                }
                else
                {
                    // read incoming bytes
                    ssize_t bytesRead = sock.readRaw(i, in_buffer[i]);

                    // client disconnect
                    if (bytesRead <= 0)
                    {
                        in_buffer.erase(i);
                        FD_CLR(i, &master_set);
                        close(i);                        
                        // break;
                    }

                    RedClone::Resp command;
                    while (RedClone::Resp::parser(in_buffer[i], command))
                    {
                        RedClone::Resp response = RedClone::executeCommand(command, kv_store);
                        string toSend = response.serialize();
                        sock.writeRaw(i, toSend);
                    }                    
                }
            }
        }
    };
}
