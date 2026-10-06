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
#include <algorithm>

using namespace std;
namespace RedClone
{
    // this class will manage its own instance of m_store; allowing for multiple stores,
    // threaded execution, etc.
    class KVStore
    {
        unordered_map<string, string> m_store{};
    public:
        enum class Command
        {
            Unknown,
            Ping,
            Set,
            Get,
            Del
        };
        
        KVStore() = default;

        // creates a copy of cmd because it transforms it in place; 
        // it needs a new mem location otherwise command gets fked
        static Command parseCommand(string cmd);

        void set(const string &key, const string &value);

        // remember to pass in output val
        bool get(const string &key, string &outValue);

        bool del(const string &key);

    private:
        inline static const unordered_map<string, Command> lookup = {
            {"PING", Command::Ping},
            {"SET", Command::Set},
            {"GET", Command::Get},
            {"DEL", Command::Del}};
    };
}

#endif