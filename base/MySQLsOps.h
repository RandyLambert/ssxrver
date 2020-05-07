#ifndef SSXERVER_BASE_MYSQLSOPS_H
#define SSXERVER_BASE_MYSQLSOPS_H
#include <string>
namespace ssxrver
{
namespace base
{
namespace SQLs
{
using std::string;
string sqlRegister(const string &str);
string sqlLogin(const string &str);

} // namespace SQLs
} // namespace base
} // namespace ssxrver

#endif
