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
    if (connect(fd, sock.getRes()->ai_addr, sock.getRes()->ai_addrlen))
    {
        die("Connect failure");
    }

    string input{};
    string in_buffer{};

    while (true)
    {
        cout << "Write message:" << endl;
        if (!getline(cin, input))
        {
            break;
        }
        RedClone::Resp resp = RedClone::Resp::fromCli(input);
        string serialized = resp.serialize();
        // send message
        sock.writeRaw(fd, serialized);
        RedClone::Resp reply;
        while (!RedClone::Resp::parser(in_buffer, reply))
        {
            // read server response
            ssize_t bytesRead = sock.readRaw(fd, in_buffer);
        }
        const auto &args = reply.getData();
        cout << "Server reply:";

        for (size_t i = 0; i < args.size(); ++i)
        {
            cout << " " << args[i] << " ";
        }
        cout << endl;
    }   

    close(fd);
};
