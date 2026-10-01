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

using namespace std;

namespace RedClone
{
    // scoped enum class with explicit char values matching the RESP protocol
    enum class RespType : char
    {
        Undef = 0,
        SimpleString = '+',
        Error = '-',
        Integer = ':',
        BulkString = '$',
        Array = '*'
    };

    class Resp
    {
        RespType type = RespType(0);

    public:
        
        Resp();


    };
}
#endif