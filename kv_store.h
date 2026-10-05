#ifndef KV_STORE_H
#define KV_STORE_H
// --- Universal C++ Network Boilerplate ---
#include <iostream>
#include <string>
#include <sstream>
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
#include <unordered_map>

using namespace std;

class KVStore{
    unordered_map<std::string, std::string> store{};

    
};

#endif