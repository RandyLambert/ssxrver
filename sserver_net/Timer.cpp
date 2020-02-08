// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/sserver/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "Timer.h"

using namespace sserver;
using namespace sserver::net;

AtomicInt64 Timer::s_numCreated_;

void Timer::restart(Timestamp now) //重启
{
  if (repeat_)
  {                                        //重新计算下一个超时时刻
    expiration_ = addTime(now, interval_); //是一个全局函数
  }
  else
  {
    expiration_ = Timestamp::invalid();
  }
}
