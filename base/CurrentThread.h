#ifndef SSXRVER_BASE_CURRENTTHREAD_H
#define SSXRVER_BASE_CURRENTTHREAD_H
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
namespace ssxrver
{
namespace CurrentThread
{
extern __thread int t_cachedTid;
extern __thread char t_tidString[32];
extern __thread int t_tidStringLength;
extern __thread const char *t_threadName;

inline int tid()
{
    if (t_cachedTid == 0)
    {
        t_cachedTid = static_cast<pid_t>(syscall(SYS_gettid));
        t_tidStringLength = snprintf(t_tidString, sizeof(t_tidString), "%5d ", t_cachedTid);
    }
    return t_cachedTid;
}

inline int tidStringLength() { return t_tidStringLength; }
inline const char *tidString() { return t_tidString; }
inline const char *name() { return t_threadName; }
bool isMainThread();

} // namespace CurrentThread

} // namespace ssxrver
#endif
