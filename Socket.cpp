#include "Socket.hpp"

RedClone::Socket::Socket(int &fd, const struct addrinfo &hints, const char *ip, const char *port) : hints{hints}
{
    int status = getaddrinfo(ip, port, &this->hints, &res);
    if (status != 0 || !res)
    {
        die("getaddrinfo failed");
    }
    const void *ipaddr = nullptr;
    if (res->ai_family == AF_INET)
    {
        char ip2[INET_ADDRSTRLEN];
        ipaddr = &reinterpret_cast<sockaddr_in *>(res->ai_addr)->sin_addr;
        inet_ntop(res->ai_family, ipaddr, ip2, sizeof(ip2));
        cout << ip2 << endl;
    }
    else if (res->ai_family == AF_INET6)
    {
        char ip2[INET6_ADDRSTRLEN];
        ipaddr = &reinterpret_cast<sockaddr_in6 *>(res->ai_addr)->sin6_addr;
        inet_ntop(res->ai_family, ipaddr, ip2, sizeof(ip2));
        cout << ip2 << endl;
    }

    fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    if (fd < 0)
    {
        die("Socket failure");
    }
}

const addrinfo *RedClone::Socket::getRes() const
{
    return this->res;
}

ssize_t RedClone::Socket::readMsg(int &fd) const
{
    // 1. read message length prefix always uint32_t long
    ssize_t bytesRead = -1;
    uint32_t msgLen = 0;
    bytesRead = read(fd, &msgLen, 4);
    msgLen = ntohl(msgLen);

    // 2. read message
    if (msgLen > 0)
    {
        char s[msgLen + 1]{};
        bytesRead = read(fd, &s, msgLen);
        s[msgLen] = '\0';
        cout << "Message: " << s << endl;
    }
    return bytesRead;
}

ssize_t RedClone::Socket::readRaw(int fd, std::string &in_buffer)
{
    char buf[12];
    ssize_t bytesRead = read(fd, buf, sizeof(buf));
    if (bytesRead > 0)
    {
        in_buffer.append(buf, bytesRead);
    }
    return bytesRead;
}



ssize_t RedClone::Socket::writeMsg(int &fd) const
{
    std::string msg{};
    cout << "Write message:" << endl;
    getline(cin, msg);
    // resp
    RedClone::Resp resp = Resp::fromCli(msg);
    string msgToSend = resp.serialize();
    cout << "result of serializing: " << msgToSend << endl;
    // resp ends

    // uint32_t msgSize = htonl(strlen(msg.c_str()));

    // // storing raw char buffer
    // char msgToSend[sizeof(msgSize) + msg.size()]{};

    // // copy msg size
    // memcpy(&msgToSend, &msgSize, sizeof(msgSize));

    // // copy message
    // memcpy(&(msgToSend[sizeof(msgSize)]), msg.c_str(), strlen(msg.c_str()));

    // send message to server
    // return write(fd, msgToSend, (size_t)sizeof(msgSize) + msg.size());
    return write(fd, msgToSend.data(), msgToSend.size());
}

ssize_t RedClone::Socket::writeRaw(int fd, const string &data)
{
    return write(fd, data.data(), data.size());
}