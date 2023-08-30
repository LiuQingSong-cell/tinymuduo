#include "TcpServer.h"
#include "logger.h"
#include "TcpConnection.h"

#include <cstring>

static EventLoop* CheckLoopNotNull(EventLoop* loop)
{
    if (loop == nullptr)
    {
        LOG_FATAL("%s %s %s mainloop is null! \n", __FILE__, __FUNCTION__, __LINE__);
    }
    return loop;
}

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr, const std::string& nameArg, Option option) : 
    loop_(CheckLoopNotNull(loop)),
    ipPort_ (listenAddr.toIpPort()),
    name_(nameArg),
    acceptor_(new Accetptor(loop, listenAddr, option == kReusePort)),
    threadpool_(new EventLoopThreadPool(loop, nameArg)),
    connectionCallback_(),
    messageCallback_(),
    nextConnId_(0),
    started_(0)
{
    // 当有新连接时 acceptor最终会调用设置的这个回调：轮询选择subloop 唤醒subloop 把connfd封装成channel分发给subloop
    acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer()
{
    for (auto& item : connections_)
    {
        TcpConnectionPtr conn(item.second);
        item.second.reset();
        conn->getLoop()->runInLoop(
            std::bind(&TcpConnection::connectDestroyed, conn)
        );
    }
}

void TcpServer::setThreadNum(int numThreads)
{
    threadpool_->setThreadNum(numThreads);
}

void TcpServer::start()
{
    if (started_++ == 0)
    {
        threadpool_->start(threadInitCallback_);
        loop_->runInLoop(std::bind(&Accetptor::listen, acceptor_.get())); 
        // 是在用户代码上开启的mainloop事件循环
    }
}

// 有一个新的客户端连接的时候，acceptor会执行这个回调操作
void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr)
{
    EventLoop* ioLoop = threadpool_->getNextLoop();
    char buf[64] = {0};
    snprintf(buf, sizeof buf, "-%s#%d", ipPort_.c_str(), nextConnId_);
    nextConnId_++;
    std::string connName = name_ + buf;
    LOG_INFO("TcpServer::newConnection [%s] - new connection [%s] form %s \n",
        name_.c_str(), connName.c_str(), peerAddr.toIpPort().c_str());
    
    sockaddr_in local;
    bzero(&local, sizeof local);
    socklen_t addLen = sizeof local;
    if (::getsockname(sockfd, (sockaddr*)&local, &addLen) < 0)
    {
        LOG_ERROR("Sockets::getLocalAddr");
    }

    InetAddress localAddr(local);

    // 根据连接成功的connfd 创建TcpConnetion
    TcpConnectionPtr conn(new TcpConnection(
        ioLoop,
        connName,
        sockfd,
        localAddr,
        peerAddr
    ));

    connections_[connName] = conn;
    // 下面的回调由用户设置给TcpServer => TcpConnection => poller => 通知channel调用回调
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);

    // 设置了如何关闭连接的回调
    conn->setCloseCallback(
        std::bind(&TcpServer::removeConnection, this, std::placeholders::_1)
    );
    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}


void TcpServer::removeConnection(const TcpConnectionPtr& conn)
{
    loop_->runInLoop(
        std::bind(&TcpServer::removeConnectionInLoop, this, conn)
    );
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn)
{
    LOG_INFO("TcpServer::removeConnectionInLoop [%s] - connection %s \n", name_.c_str(), conn->name().c_str());

    connections_.erase(conn->name());
    EventLoop* ioLoop = conn->getLoop();
    ioLoop->queueInLoop(
        std::bind(&TcpConnection::connectDestroyed, conn)
    );
}
