#ifndef RESP_H
#define RESP_H
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

    // A client sends to the Redis server a RESP Array consisting of just Bulk Strings.
    // A Redis server replies to clients sending any valid RESP data type as reply.
    class Resp
    {
        RespType type = RespType::Undef;
        vector<string> data{};

    public:
        Resp() = default;
        // Construct simple primitives (SimpleString, Error, Integer, BulkString)
        // mostly for server
        Resp(RespType type, string val);

        // Construct RESP Array from a vector of strings
        // use case: Resp(tokenize("SET key value"))
        explicit Resp(vector<string> items);

        // Factory constructor: Parses raw CLI line ("SET key value") into a RESP Array object
        static Resp fromCli(const string &cliInput);

        // serializes client messages (\r\n, etc.)
        string serialize() const;

        // static because it doesn't require member variables
        // allows for calling without an instance
        static vector<string> tokenize(const string &msg);

        static bool parser(string &buf, Resp &outResp);

        static size_t findCRLF(const string &buf, size_t pos = 0);

        // GETTERS
        // returns unmodifiable
        const vector<string> &getData() const;

        RespType getType() const;
    };
}
#endif