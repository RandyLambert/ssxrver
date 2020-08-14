#include <sys/epoll.h>
#include "EventLoop.h"
#include "Channel.h"
#include "TcpConnection.h"
#include "../base/Logging.h"
using namespace ssxrver;
using namespace ssxrver::net;

namespace ssxrver::net
{
    const unsigned kNoneEvent = 0; //初始化为默认值
    const unsigned kReadEventLT = EPOLLIN | EPOLLPRI;
    const unsigned kReadEventET = EPOLLIN | EPOLLPRI | EPOLLET;
    const unsigned kWriteEvent = EPOLLOUT;
}

Channel::Channel(EventLoop *loop, int fd)
    : loop_(loop),
      fd_(fd),
      events_(0),
      revents_(0),
      status_(-1),
      logHup_(true),
      tied_(false),
      eventHandling_(false),
      addedToLoop_(false)
{
}

Channel::~Channel()
{
    assert(!eventHandling_);
    assert(!addedToLoop_);
    close(fd_);
}

std::shared_ptr<TcpConnection> Channel::getTie()
{
    std::shared_ptr<TcpConnection> temp(tie_.lock());
    return temp;
}
void Channel::tie(std::shared_ptr<TcpConnection> &obj)
{
    tie_ = obj;
    tied_ = true;
}

void Channel::update()
{
    addedToLoop_ = true;
    loop_->updateChannel(this); //调用loop的update，loop的update又调用了channel的update
}

void Channel::remove()
{
    assert(isNoneEvent());
    addedToLoop_ = false;
    loop_->removeChannel(this);
}

void Channel::handleEvent()
{
    std::shared_ptr<void> guard;
    if (tied_)
    {
        guard = tie_.lock(); //weak_ptr的使用
        if (guard)
        {
            handleEventWithGuard();
        }
    }
    else
    {
        handleEventWithGuard();
    }
}

void Channel::handleEventWithGuard()
{
    eventHandling_ = true;
    if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) //判断一下返回的事件，进行处理
    {
        if (logHup_)
        {
            LOG_WARN << "Channel::handle_event() EPOLLHUP";
        }
        if (closeCallback_)
            closeCallback_();
    }
    if (revents_ & EPOLLERR)
    {
        if (errorCallback_)
            errorCallback_();
    }
    if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLHUP)) //可读事件，最后一个是等待方关闭连接
    {
        if (readCallback_)
        {
            readCallback_();
        }
    }
    if (revents_ & EPOLLOUT)
    {
        if (writeCallback_)
            writeCallback_();
    }
    eventHandling_ = false;
}
