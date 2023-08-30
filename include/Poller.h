#ifndef POLLER_H__
#define POLLER_H__

#include "noncopyable.h"
#include <vector>
#include <unordered_map>
#include "Timestamp.h"

class Channel;
class EventLoop;

class Poller : noncopyable
{
    public:
        using ChannelList = std::vector<Channel*>;
        virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels) = 0;
        virtual void updateChannel(Channel* channel) = 0;
        virtual void removeChannel(Channel* channel) = 0;
        virtual bool hasChannel(Channel* channel) const;

        // get the special poll/epoll
        static Poller* newDefaultPoller(EventLoop* loop);
    
    Poller(EventLoop* loop);
    virtual ~Poller();

    protected:
        using ChannelMap = std::unordered_map<int, Channel*>;
        ChannelMap Channels_;
    
    private:
        EventLoop* ownerLoop_;
};




#endif