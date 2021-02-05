#include "File.h"
#include <cstdio>
#include <boost/assert.hpp>
#include <unistd.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include<sys/stat.h>
using namespace ssxrver;
using namespace ssxrver::base::file;

WriteFile::WriteFile(std::string_view fileName)
    : fp_(fopen(fileName.data(), "ae")),
      writeLen_(0)
{
    int err = ferror(fp_);
    BOOST_ASSERT_MSG(err,"WriteFile error");
    setbuffer(fp_, buffer_, sizeof buffer_);
}
WriteFile::~WriteFile()
{
    flush();
    ::fclose(fp_);
}
void WriteFile::append(const char *log_, const size_t len)
{
    size_t n = 0;
    size_t remain = len;
    do
    {
        size_t x = this->write(log_ + n, remain);
        if (x == 0)
        {
            int err = ferror(fp_);
            if (err)
                fprintf(stderr, "WriteFile error");
            break;
        }

        n += x;
        remain = len - n;

    } while (remain > 0);

    writeLen_ += len;
}

ReadSmallFile::ReadSmallFile(std::string_view fileName)
:fd_(::open(fileName.data(), O_RDONLY | O_CLOEXEC)),
err_(0)
{
    buf_[0] = '\0';
    if (fd_ < 0)
    {
        err_ = errno;
    }
}

int ReadSmallFile::readToBuffer(ssize_t *size) //size传入传出参数,返回值是错误代码
{
    int err = err_;
    if(fd_ >= 0)
    {
        ssize_t n = ::pread(fd_,buf_, sizeof(buf_)-1,0);
        if(n >= 0)
        {
            if(size)
            {
                *size = static_cast<ssize_t>(n);
            }
            buf_[n] = '\0';
        }
        else
        {
            err = errno;
        }
    }
    return err;
}
//
//int ReadSmallFile::readToString(string &data){
//
//}


ReadSmallFile::~ReadSmallFile()
{
    if(fd_ >= 0){
        ::close(fd_);
    }
}

SendFileUtil::SendFileUtil(std::string_view fileName):
    inFd_(::open(fileName.data(), O_RDONLY | O_CLOEXEC)),
    offset_(0),
    err_(0) {
    if (inFd_ < 0)
    {
        err_ = errno;
    } else {
        struct stat statBuf{};
        fstat(inFd_,&statBuf);
        sendLen_ = static_cast<size_t>(statBuf.st_size);
    }
}

SendFileUtil::~SendFileUtil()
{
    if(inFd_ >= 0) {
//        LOG_DEBUG<< " close " <<inFd_;
        ::close(inFd_);
    }
}

SendFileUtil::SendFileUtil() : inFd_(-1),
offset_(0),
sendLen_(0),
err_(0) {
}