#ifndef SSXRVER_BASE_NONCOPYABLE_H
#define SSXRVER_BASE_NONCOPYABLE_H
namespace ssxrver
{

class noncopyable
{
public:
    noncopyable(const noncopyable &) = delete;
    void operator=(const noncopyable &) = delete;

protected:
    noncopyable() = default;
    ~noncopyable() = default;
};

} // namespace ssxrver
#endif
