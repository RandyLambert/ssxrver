#ifndef SSXRVER_BASE_NONECOPYABLE_H
#define SSXRVER_BASE_NONECOPYABLE_H
namespace ssxrver
{

class nonecopyable
{
public:
    nonecopyable(const nonecopyable&) = delete;
    void operator=(const nonecopyable&) = delete;

protected:
    nonecopyable()=default;
    ~nonecopyable()=default;
};

}
#endif

