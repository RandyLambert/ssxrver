#ifndef SSXRVER_BASE_FILE_H
#define SSXRVER_BASE_FILE_H
#include <string>
#include <stdio.h>
#include "noncopyable.h"
namespace ssxrver
{
namespace base
{
namespace file
{ //封装文件操作
using std::string;
class writeFile : noncopyable
{
public:
    writeFile(const string &fileName);
    ~writeFile();
    void append(const char *log_, size_t len);
    void flush() { ::fflush(fp_); };
    size_t writeLen() { return writeLen_; }

private:
    size_t write(const char *log_, size_t len) { return ::fwrite_unlocked(log_, 1, len, fp_); }
    FILE *fp_;
    size_t writeLen_;
    char buffer_[64 * 1024];
};

} // namespace file
} // namespace base
} // namespace ssxrver

#endif
