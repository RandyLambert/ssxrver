#ifndef SSXRVER_BASE_FILE_H
#define SSXRVER_BASE_FILE_H
#include <string>
#include <boost/noncopyable.hpp>
namespace ssxrver::base::file
{ //封装文件操作
using std::string;
static const int kFileBufferSize = 64 * 1024;

class writeFile : boost::noncopyable
{
public:
    explicit writeFile(std::string_view fileName);
    ~writeFile();
    void append(const char *log_, size_t len);
    void flush() { ::fflush(fp_); };
    [[nodiscard]] size_t writeLen() const { return writeLen_; }

private:
    size_t write(const char *log_, size_t len) { return ::fwrite_unlocked(log_, 1, len, fp_); }
    FILE *fp_;
    size_t writeLen_;
    char buffer_[kFileBufferSize];
};

// read small file < 64KB
class ReadSmallFile : boost::noncopyable
{
public:
    explicit ReadSmallFile(std::string_view fileName);
    ~ReadSmallFile();

//    int readToString(string& data);

    // return errno
    int readToBuffer(ssize_t *size);

    [[nodiscard]] const char* buffer() const { return buf_; }

private:
    int fd_;
    int err_;
    char buf_[kFileBufferSize];
};

} // namespace ssxrver::base::file

#endif
