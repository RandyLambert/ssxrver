#ifndef SSXRVER_NET_BUFFER_H
#define SSXRVER_NET_BUFFER_H
#include <string>
#include <vector>
namespace ssxrver
{
namespace net
{
using std::string;
class Buffer
{
public:
    static const size_t Kprepend = 8;
    static const size_t KinitSize = 1024;
    Buffer() {}
    ~Buffer() {}

private:

};

}
}
#endif
