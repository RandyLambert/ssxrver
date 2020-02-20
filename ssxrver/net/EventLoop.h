#ifndef SSXRVER_NET_EVENTLOOP_H
#define SSXRVER_NET_EVENTLOOP_H
#include <vector>
#include <any>
#include <memory>
#include <functional>
#include <atomic>
#include "../base/noncopyable.h"
#include "../base/CurrentThread.h"
#include "../base/Mutex.h"
namespace ssxrver
{
namespace net
{
class Channel;
class Epoller;

class EventLoop : noncopyable
{
public:
    typedef std::function<void()> Functor;
    EventLoop();
    ~EventLoop();
    void loop();
    void quit();

    void runInLoop(Functor cb);
    void queueInLoop(Functor cb);


private:

};

}
}
#endif
