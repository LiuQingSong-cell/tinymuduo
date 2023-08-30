#pragma once

#include "noncopyable.h"
#include "EventLoopThread.h"

#include <string>
#include <vector>
#include <memory>

class EventLoop;

class EventLoopThreadPool
{
    public:
        using ThreadInitCallback = std::function<void(EventLoop*)>;

        EventLoopThreadPool(EventLoop* baseloop, const std::string& nameArg);
        ~EventLoopThreadPool() = default;

        void setThreadNum(int numThreads) {numThreads_ = numThreads;} // TcpServer.setThreadNum --> EventLoopThreadPool.setThreadNum

        void start(const ThreadInitCallback& cb = ThreadInitCallback());

        // 如果工作在多线程 baseloop_默认以轮询的方式分配channel给subloop
        EventLoop* getNextLoop();

        std::vector<EventLoop*> getAllLoops();
        bool started() const {return started_;}
        const std::string name() const {return name_;}

    private:
        EventLoop* baseLoop_; // 如果只有一个线程
        std::string name_;
        bool started_;
        int numThreads_;
        int next_;
        std::vector<std::unique_ptr<EventLoopThread>> threads_;
        std::vector<EventLoop*> loops_;
        
};