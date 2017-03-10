#ifndef CHECKKING_RPC_NET_EVENTLOOPTHREAD_H
#define CHECKKING_RPC_NET_EVENTLOOPTHREAD_H

#include <boost/noncopyable.hpp>
#include "Mutex.h"
#include "ThreadComm.h"
#include "Condition.h"

namespace checkking {
namespace rpc {
class EventLoop;

class EventLoopThread : boost::noncopyable {
public:
    EventLoopThread();
    ~EventLoopThread();
    EventLoop* startLoop();

private:
    void threadFunc();

    EventLoop* _loop;
    bool _exiting;
    Thread _thread;
    MutexLock _mutex;
    Condition _cond;
}; // class EventLoopThread
} // namespace rpc
} // namespace checkking
#endif  // CHECKKING_RPC_NET_EVENTLOOPTHREAD_H
