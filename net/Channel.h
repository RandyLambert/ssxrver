#ifndef SSXRVER_NET_CHANNEL_H
#define SSXRVER_NET_CHANNEL_H
#include <functional>
#include <memory>
#include <string>
#include <boost/noncopyable.hpp>
namespace ssxrver::net
{
class EventLoop;
class TcpConnection;

extern const unsigned kNoneEvent; //初始化为默认值
extern const unsigned kReadEventLT;
extern const unsigned kReadEventET;
extern const unsigned kWriteEvent;

class Channel : boost::noncopyable
{
public:
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void()>;

    explicit Channel(EventLoop *loop, int fd); //一个eventLoop可能会包含多个channel，但一个channel只能包含一个eventloop
    ~Channel();

    void handleEvent(); //重点，执行epoll的任务
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

    void tie(std::shared_ptr<TcpConnection> &); //和tcpconnection对象有关系，防止对象销毁
    std::shared_ptr<TcpConnection> getTie();

    [[nodiscard]] int fd() const { return fd_; }                             //channel对应的文件描述符
    [[nodiscard]] unsigned events() const { return events_; }                //channel注册了那些时间保存在events中
    void setRevents(unsigned revents) { revents_ = revents; }            //epoll
    void enableEvents(unsigned events) {events_ |= events; update(); }
    void disableEvents(unsigned events) {events_ &= ~events; update(); }
    void disableAll(){events_ = kNoneEvent; update(); }

    [[nodiscard]] bool isNoneEvent() const { return events_ == kNoneEvent; } //判断是否没有事件
    [[nodiscard]] bool isWriting() const { return events_ & kWriteEvent; }
    [[nodiscard]] bool isReading() const { return events_ & kReadEventLT; }

    [[nodiscard]] int status() const { return status_; }
    void setStatus(int status) { status_ = status; }

    void doNotLogHup() { logHup_ = false; }

    EventLoop *ownerLoop() { return loop_; }
    void remove();
    void channelReset(int socketId);
private:
    void update();
    void handleEventWithGuard();

    EventLoop *loop_; //记录所属的eventLoop
    int fd_;    //文件描述符，负责关闭
    unsigned events_;      //关注的事件
    unsigned revents_;     //epoll实际返回的事件个数
    int status_;      //epoll中通道的状态
    bool logHup_;     //for EPOLLHUP

    std::weak_ptr<TcpConnection> tie_;
    bool tied_;
    bool eventHandling_; //是否在处理事件中
    bool addedToLoop_;
    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;

};

} // namespace ssxrver::net
#endif
