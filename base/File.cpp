#include "File.h"
#include <cstdio>
#include <boost/assert.hpp>
#include <unistd.h>
#include <fcntl.h>
using namespace ssxrver;
using namespace ssxrver::base::file;

writeFile::writeFile(const string &fileName)
    : fp_(fopen(fileName.c_str(), "ae")),
      writeLen_(0)
{
    int err = ferror(fp_);
    BOOST_ASSERT_MSG(err,"writeFile error");
    setbuffer(fp_, buffer_, sizeof buffer_);
}
writeFile::~writeFile()
{
    flush();
    ::fclose(fp_);
}
void writeFile::append(const char *log_, const size_t len)
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
                fprintf(stderr, "writeFile error");
            break;
        }

        n += x;
        remain = len - n;

    } while (remain > 0);

    writeLen_ += len;
}

ReadSmallFile::ReadSmallFile(const string &fileName)
:fd_(::open(fileName.c_str(), O_RDONLY | O_CLOEXEC)),
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