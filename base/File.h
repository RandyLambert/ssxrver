/*
 * @Date: 2020-08-06 17:33:41
 * @LastEditors: OBKoro1
 * @LastEditTime: 2021-02-04 21:44:29
 * @FilePath: /ssxrver/base/File.h
 * @Auther: SShouxun
 * @GitHub: https://github.com/RandyLambert
 */
#ifndef SSXRVER_BASE_FILE_H
#define SSXRVER_BASE_FILE_H
#include <string>
#include <cassert>
#include <boost/noncopyable.hpp>
namespace ssxrver::base::file
{ //封装文件操作
using std::string;
static const int kFileBufferSize = 64 * 1024;

class WriteFile : boost::noncopyable
{
public:
    explicit WriteFile(std::string_view fileName);
    ~WriteFile();
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

    // int readToString(string& data);
    // return errno
    int readToBuffer(ssize_t *size);

    [[nodiscard]] const char* buffer() const { return buf_; }

private:
    int fd_;
    int err_;
    char buf_[kFileBufferSize];
};

class SendFileUtil
{
public:
    explicit SendFileUtil(std::string_view fileName);
    explicit SendFileUtil();
    ~SendFileUtil();
    [[nodiscard]] int getInId() const {return inFd_;}
    size_t& getSendLen() {  return sendLen_;}
    off_t* getOffset() {return &offset_; }
private:
    int inFd_;
    off_t offset_;
    size_t sendLen_;
    int err_;
};

} // namespace ssxrver::base::file

#endif
