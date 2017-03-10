#ifndef CHECKKING_RPC_BASE_COUNTDOWNLATCH_H
#define CHECKKING_RPC_BASE_COUNTDOWNLATCH_H

#include "Mutex.h"
#include "Condition.h"
#include <boost/noncopyable.hpp>

namespace checkking {
namespace rpc {

class CountDownLatch : boost::noncopyable
{
 public:

  explicit CountDownLatch(int count)
    : mutex_(),
      condition_(mutex_),
      count_(count)
  {
  }

  void wait()
  {
    MutexLockGuard lock(mutex_);
    while (count_ > 0)
    {
      condition_.wait();
    }
  }

  void countDown()
  {
    MutexLockGuard lock(mutex_);
    --count_;
    if (count_ == 0)
    {
      condition_.notifyAll();
    }
  }

  int getCount() const
  {
    MutexLockGuard lock(mutex_);
    return count_;
  }

 private:
  mutable MutexLock mutex_;
  Condition condition_;
  int count_;
};

}
}
#endif  // CHECKKING_RPC_BASE_COUNTDOWNLATCH_H
