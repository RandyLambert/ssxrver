#ifndef SSXERVER_NET_MYSQLSOPS_H
#define SSXERVER_NET_MYSQLSOPS_H
#include <string>
namespace ssxrver
{
namespace net
{
namespace SQLs
{
using std::string;
string sqlRegister(const string &str);
string sqlLogin(const string &str);

} // namespace SQLs
} // namespace net
} // namespace ssxrver

#endif
