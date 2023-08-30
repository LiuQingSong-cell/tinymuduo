#ifndef EVENTLOOP__H__
#define EVENTLOOP__H__

#include "noncopyable.h"
#include "Timestamp.h"
#include "CurrentThread.h"

#include <functional>
#include <vector>
#include <atomic>
#include <memory>
#include <mutex>

class Channel;
class Poller;

class EventLoop : noncopyable
{
    public:
        using Functor = std::function<void()>;

        EventLoop();
        ~EventLoop();

        // start event loop
        void loop();
        void quit();

        Timestamp pollReturnTime() const {return pollReturnTime_;}

        // call cb in currnt loop
        void runInLoop(Functor cb);
        // add cb to pendingFunctors_, call cb when wakeup loop thread 
        void queueInLoop(Functor cb);

        // wekeup loop thread
        void wakeup();

        void updateChannel(Channel* channel);
        void removeChannel(Channel* channel);
        bool hasChannel(Channel* channel);

        bool isInLoopThread() const {return threadId_ == CurrentThread::tid();}

    private:
        void handleRead(); // 
        void doPendingFunctor();

    private:
        using ChannelList = std::vector<Channel*>;
        std::atomic_bool looping_;
        std::atomic_bool quit_;
        const pid_t threadId_; // 

        Timestamp pollReturnTime_; // the time of poller return active channels
        std::unique_ptr<Poller> poller_;

        //system call int eventFd() 
        // whne mainloop has a new user's channel, it will notice a subloop to handle it by round-robin
        int wakeupFd_; 
        std::unique_ptr<Channel> wakeupChannel_;

        ChannelList activeChannels_;

        std::atomic_bool callingPendingFunctors_; //
        std::mutex mutex_;
        std::vector<Functor> pendingFunctors_;
};

#endif