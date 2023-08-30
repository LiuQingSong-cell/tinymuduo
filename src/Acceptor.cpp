
#include "Acceptor.h"
#include "logger.h"
#include "InetAddress.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

static int createNonblocking()
{
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (sockfd < 0) 
    {
        LOG_FATAL("%s %s %s Socket create error \n", __FILE__, __FUNCTION__, __LINE__);
    }
    return sockfd;
}

Accetptor::Accetptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport) :
    loop_(loop),
    acceptSocket_(createNonblocking()),
    acceptChannel_(loop, acceptSocket_.fd()),
    listenning_(false)
{
    // LOG_INFO()
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(true);
    acceptSocket_.bindAddress(listenAddr);
    // TcpServer::start() Acceptor.listen 有新用户链接 执行一个回调(connfd --> channel -->subloop)
    acceptChannel_.setReadCallback(std::bind(&Accetptor::handleRead, this));
}

Accetptor::~Accetptor()
{
    acceptChannel_.disableAll();
    acceptChannel_.remove();
}

void Accetptor::listen()
{
    listenning_ = true;
    acceptSocket_.listen();
    acceptChannel_.enableReading();
}

// listenfd 有事件发生 即有新链接
void Accetptor::handleRead()
{
    InetAddress peerAddr;
    int connfd = acceptSocket_.accept(&peerAddr);
    if (connfd > 0)
    {
        if (newConnectionCallback_)
            // 轮询找到subloop 唤醒 分发给他新连接的channel
            newConnectionCallback_(connfd, peerAddr);
        else
            ::close(connfd);
    }
    else
    {
        LOG_ERROR("Acceptor handleRead error \n");
    }
}