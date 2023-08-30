#ifndef TCPSERVER__H__
#define TCPSERVER__H__

#include "noncopyable.h"
#include "EventLoop.h"
#include "Acceptor.h"
#include "InetAddress.h"
#include "EventLoopThreadPool.h"
#include "Callbacks.h"


#include <functional>
#include <string>
#include <memory>
#include <atomic>
#include <unordered_map>

class TcpServer : noncopyable
{
    public:
        using ThreadInitCallback = std::function<void(EventLoop*)>;

        enum Option
        {
            kNoReusePort,
            kReusePort
        };

        TcpServer(EventLoop* loop, const InetAddress& listenAddr, const std::string& nameArg, Option option = kReusePort);
        ~TcpServer();

        void setThreadInitCallback(const ThreadInitCallback& cb) {threadInitCallback_ = std::move(cb);}
        void setConnectionCallback(const ConnectionCallback& cb) {connectionCallback_ = std::move(cb);}
        void setWriteCompleteCallback(const WriteCompleteCallback& cb) {writeCompleteCallback_ = std::move(cb);}
        void setMessageCallback(const MessageCallback& cb) {messageCallback_ = std::move(cb);}

        void setThreadNum(int numThreads); // 设置subloop个数

        void start();
    
    private:
        void newConnection(int sockfd, const InetAddress& peerAddr);
        void removeConnection(const TcpConnectionPtr& conn);
        void removeConnectionInLoop(const TcpConnectionPtr& conn);
    
    private:
        using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;

        EventLoop* loop_; // the acceptor loop
        const std::string ipPort_;
        const std::string name_;
        std::unique_ptr<Accetptor> acceptor_; // 运行在mainloop 监听新连接
        std::shared_ptr<EventLoopThreadPool> threadpool_;

        ConnectionCallback connectionCallback_;
        MessageCallback messageCallback_;
        WriteCompleteCallback writeCompleteCallback_;

        ThreadInitCallback threadInitCallback_; // loop线程初始化的回调
        std::atomic_int started_;

        int nextConnId_;
        ConnectionMap connections_; // 所有连接
};

#endif