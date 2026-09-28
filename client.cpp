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
static int32_t query(int fd, const char *text)
{
}

int main()
{
    struct addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    struct addrinfo *res;
    getaddrinfo("127.0.0.1", "1234", &hints, &res);
    char ip[INET_ADDRSTRLEN];
    // inet_ntop(AF_INET, reinterpret_cast<sockaddr_in *>(res->ai_addr)->sin_addr, ip, INET_ADDRSTRLEN);
    inet_ntop(AF_INET, &reinterpret_cast<sockaddr_in *>(res->ai_addr)->sin_addr, ip, INET_ADDRSTRLEN);

    cout << ip << endl;

    // socket
    int fd = socket(hints.ai_family, hints.ai_socktype, hints.ai_protocol);
    if (fd < 0)
    {
        die("Socket failure");
    }

    if (connect(fd, res->ai_addr, res->ai_addrlen))
    {
        die("Connect failure");
    }

    std::string msg{};
    int bWritten = -2;
    int maxBytes = 4096;
    char s[maxBytes];
    int bytesRed = -2;
    do
    {        
        cout << "Write message:" << endl;
        getline(cin, msg);
        bWritten = write(fd, msg.c_str(), strlen(msg.c_str()));

        if (bWritten <= 0)
        {
            break;
        }

        bytesRed = read(fd, s, maxBytes);
        if (bytesRed > 0 && bytesRed < maxBytes)
        {
            s[bytesRed] = '\0';
            cout << "Server sent " << bytesRed << " bytes. Message: " << s << endl;
        }

    } while (bWritten > 0);
};
