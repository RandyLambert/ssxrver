#ifndef SSXRVER_NET_CHANNEL_H
#define SSXRVER_NET_CHANNEL_H
#include <functional>
#include <memory>
#include "../base/noncopyable.h"
namespace ssxrver
{
namespace net
{
class EventLoop;

class Channel : noncopyable
{
public:
    typedef std::function<void()>EventCallback;
    typedef std::function<void()>ReadEventCallback;

    Channel(EventLoop *loop,int fd) ; //一个eventloop可能会包含多个channel，但一个channel只能包含一个channel
    ~Channel();

    void handleEvent();//重点，执行epoll的任务
    void setReadCallback(ReadEventCallback cb)
    {
        readCallback_ = std::move(cb);
    }
    void setWriteCallback(EventCallback cb)
    {
        writeCallback_ = std::move(cb);
    }
    void setCloseCallback(EventCallback cb)
    {
        closeCallback_ = std::move(cb);
    }
    void setErrorCallback(EventCallback cb)
    {
        errorCallback_ = std::move(cb);
    }

    void tie(const std::shared_ptr<void> &); //和tcpconnection对象有关系，防止对象销毁

    int fd() const { return fd_; }                  //channel对应的文件描述符
    int events() const { return events_; }          //channel注册了那些时间保存在events中
    void set_revents(int revt) { revents_ = revt; } //epoll
    bool isNoneEvent() const { return events_ == kNoneEvent; }//判断是否没有事件
    void enableReading() //关注读事件，或者加入这个事件
    {
        events_ |= kReadEvent;
        update();
    }

    void disableReading()
    {
        events_ &=~kReadEvent;
        update();
    }
    void enableWriting()
    {
        events_ |= kWriteEvent;
    }
    void disableWriting()
    {
        events_ |= kWriteEvent;
    }
    void disableAll()//不关注事件了
    {
        events_ = kNoneEvent;
    }

    bool isWriting() const { return events_ & kWriteEvent; }
    bool isReading() const { return events_ & kReadEvent; }

    int status() { return status_; }
    void set_status(int idx) { status_ = idx; }
    std::string reventsToString() const;

    void doNotLogHup() { logHup_ = false; }
    
    EventLoop *ownerLoop() { return loop_; }
    void remove();
private:
    void update();
    void handleEventWithGuard();
    
    static const int kNoneEvent;  //没有关注事件
    static const int kReadEvent;  //POLLIN | POLLPRI（紧急事件），默认LT
    static const int kWriteEvent; //POLLOUT写

    EventLoop *loop_; //记录所属的eventloop
    const int fd_;    //文件描述符，可能负责关闭
    int events_;      //关注的事件
    int revents_;      //epoll实际返回的事件个数
    int status_;        //epoll中通道的状态
    bool logHup_;     //for EPOLLHUP

    std::weak_ptr<void> tie_;
    bool tied_;
    bool eventHandling_; //是否在处理事件中
    bool addedToLoop_;
    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;

};

}
}
#endif
