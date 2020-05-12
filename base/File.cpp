#include "File.h"
#include <stdio.h>
using namespace ssxrver;
using namespace ssxrver::base::file;

writeFile::writeFile(const string &fileName)
    : fp_(fopen(fileName.c_str(), "ae")),
      writeLen_(0)
{
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
