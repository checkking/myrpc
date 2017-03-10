#ifndef CHECKKING_RPC_NET_EVENTLOOPTHREADPOOL_H
#define CHECKKING_RPC_NET_EVENTLOOPTHREADPOOL_H

#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include "EventLoopThread.h"

namespace checkking {
namespace rpc {

class EventLoop;
class EventLoopThreadPool : boost::noncopyable {
public:
    EventLoopThreadPool(EventLoop* baseLoop)
    : _baseLoop(baseLoop),
    _started(false),
    _next(0),
    _numThreads(0) {
    }
    ~EventLoopThreadPool() {
    }
    void setThreadNum(int numThreads) {
        _numThreads = numThreads;
    }
    void start();

    EventLoop* getNextLoop();
private:
    EventLoop* _baseLoop;
    bool _started;
    int _next;
    int _numThreads;
    boost::ptr_vector<EventLoopThread> _threads;
    std::vector<EventLoop*> _loops;
}; // class EventLoopThreadPool
} // namespace rpc
} // namespace checkking
#endif  // CHECKKING_RPC_NET_EVENTLOOPTHREADPOOL_H
