#pragma once

#include "noncopyable.h"
#include "Channel.h"
#include "Socket.h"
#include <functional>

class EventLoop;
class InetAddress;

class Accetptor
{
    public:
        using NewConnectionCallback = std::function<void(int sockfd, const InetAddress&)>;

        Accetptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport);
        ~Accetptor();

        void setNewConnectionCallback(NewConnectionCallback cb)
        {
            newConnectionCallback_ = cb;
        }
        
        bool listenning() const { return listenning_;}
        void listen();

    private:
        void handleRead();

    private:
        EventLoop* loop_; // 用户定义的那个baseloop 也叫mainloop
        Socket acceptSocket_;
        Channel acceptChannel_;
        NewConnectionCallback newConnectionCallback_;
        bool listenning_;
};