// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is a public header file, it must only include public header files.

#ifndef SSERVER_PROCESSINFO_H
#define SSERVER_PROCESSINFO_H

#include "StringPiece.h"
/* #include "Types.h" */
#include "Timestamp.h"
#include <vector>

namespace sserver
{

namespace ProcessInfo
{
pid_t pid();
std::string pidString();
uid_t uid();
std::string username();
uid_t euid();
Timestamp startTime();
int clockTicksPerSecond();
int pageSize();
bool isDebugBuild(); // constexpr

std::string hostname();
std::string procname();
StringPiece procname(const std::string &stat);

/// read /proc/self/status
std::string procStatus();

/// read /proc/self/stat
std::string procStat();

/// read /proc/self/task/tid/stat
std::string threadStat();

/// readlink /proc/self/exe
std::string exePath();

int openedFiles();
int maxOpenFiles();

struct CpuTime
{
  double userSeconds;
  double systemSeconds;

  CpuTime() : userSeconds(0.0), systemSeconds(0.0) {}
};
CpuTime cpuTime();

int numThreads();
std::vector<pid_t> threads();
} // namespace ProcessInfo

} // namespace sserver

#endif // SSERVER_PROCESSINFO_H
