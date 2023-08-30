#ifndef EPOLLPOLLER_H__
#define EPOLLPOLLER_H__

#include "Poller.h"
#include <vector>
#include <sys/epoll.h>
#include <unistd.h>

class EpollPoller : public Poller
{
    public:
        EpollPoller(EventLoop* loop);
        ~EpollPoller() override;

        Timestamp poll(int timeoutMs, ChannelList* activeChannels) override;
        void updateChannel(Channel* channel) override;
        void removeChannel(Channel* channel) override;

    private:
        using EventList = std::vector<epoll_event>;

        static const int kInitEventListSize = 16;

        void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;
        void update(int operation, Channel* channel);

        int epollfd_;
        EventList events_;

};


#endif