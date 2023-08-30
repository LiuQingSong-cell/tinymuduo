#ifndef CHANNEL__H__
#define CHANNEL__H__

#include "noncopyable.h"
#include "Timestamp.h"
#include <functional>
#include <memory>

class EventLoop; // forward declaration 

class Channel : noncopyable
{
    public:
        using EventCallback = std::function<void()>;
        using ReadEventCallback = std::function<void(Timestamp)>;

        // handle events that happen on fd
        void handleEvent(Timestamp receiveStamp);

        void setReadCallback(ReadEventCallback cb)
        {readCallback_ = std::move(cb);}

        void setWriteCallback(EventCallback cb)
        {writeCallback_ = std::move(cb);}

        void setCloseCallback(EventCallback cb)
        {closeCallback_ = std::move(cb);}

        void setErrorCallback(EventCallback cb)
        {errorCallback_ = std::move(cb);}

        // avoid to process callback after removing TcpConnection
        void tie(const std::shared_ptr<void>&);

        int fd() const {return fd_;}

        int events() const {return events_;}

        void set_revents(int revt) {revents_ = revt;}

        bool isNoneEvent() const {return events_ == kNoneEvent;}

        void enableReading() { events_ |= kReadEvent; update(); }
        void disableReading() { events_ &= ~kReadEvent; update(); }
        void enableWriting() { events_ |= kWriteEvent; update(); }
        void disableWriting() { events_ &= ~kWriteEvent; update(); }
        void disableAll() { events_ = kNoneEvent; update(); }
        bool isWriting() const { return events_ & kWriteEvent; }
        bool isReading() const { return events_ & kReadEvent; }

        int index() const {return index_;}
        void set_index(int idx) {index_ = idx;}

        EventLoop* ownerloop() const {return loop_;}

        void remove();

        Channel(EventLoop* loop, int fd);
        ~Channel();


    
    private:
        void update();
        void handleEventWithGuard(Timestamp receiveTime);


    private:
        EventLoop* loop_; // who own this channel
        int fd_;          // responsible for a file descriptor
        int events_;      // register events of interest for fd
        int revents_;     // // it's the received event types of epoll or poll
        int index_;       // used by Poller.

        std::weak_ptr<void> tie_;  // TcpConnection => channel 执行回调的时候用于判断对应的TcpConnection是否有效

        static const int kNoneEvent;
        static const int kReadEvent;
        static const int kWriteEvent;

        bool tied_;
        bool eventHandling_;
        bool addedToLoop_;
        // following callbacks are passed by user
        ReadEventCallback readCallback_;
        EventCallback writeCallback_;
        EventCallback closeCallback_;
        EventCallback errorCallback_;
};

#endif