#include "Socket.h"
#include "InetAddress.h"
#include "logger.h"

#include <cerrno>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/tcp.h> // TCP_NODELAY


Socket::~Socket()
{
    ::close(sockfd_);
}

void Socket::bindAddress(const InetAddress& localaddr)
{
    if (::bind(sockfd_, (sockaddr*)localaddr.getSockAddr(), sizeof(sockaddr_in)) != 0)
    {
        LOG_FATAL("bind sockfd:%d fail, errno is %d \n", sockfd_, errno);
    }
}

void Socket::listen()
{
    if (::listen(sockfd_, 1024) != 0)
    {
        LOG_FATAL("listen sockfd:%d fail \n", sockfd_);
    }
}

int Socket::accept(InetAddress* peeraddr)
{
    sockaddr_in addr;
    bzero(&addr, sizeof addr);

    socklen_t len = sizeof addr;
    int connsockfd = ::accept4(sockfd_, (sockaddr*)&addr, &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (connsockfd >= 0)
    {
        peeraddr->setSockAddr(addr);
    }

    return connsockfd;
}

void Socket::shutdonewrite()
{
    if (::shutdown(sockfd_, SHUT_WR) != 0)
    {
        LOG_ERROR("shutdown write error for sockfd: %d", sockfd_);
    }
}

void Socket::setTcpNoDelay(bool on)
{
    int optval = on? 1 : 0;
    ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &optval, (socklen_t)(sizeof optval));
}

void Socket::setReuseAddr(bool on)
{
    int optval = on? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, (socklen_t)(sizeof optval));
}

void Socket::setReusePort(bool on)
{
    int optval = on? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &optval, (socklen_t)(sizeof optval));
}

void Socket::setKeepAlive(bool on)
{
    int optval = on? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval, (socklen_t)(sizeof optval));
}