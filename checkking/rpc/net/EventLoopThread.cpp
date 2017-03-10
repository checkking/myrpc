#include "EventLoopThread.h"
#include "EventLoop.h"
#include <boost/bind.hpp>

namespace checkking {
namespace rpc {
EventLoopThread::EventLoopThread():
    _loop(NULL),
    _exiting(false),
    _thread(boost::bind(&EventLoopThread::threadFunc, this)),
    _mutex(),
    _cond(_mutex) {}

EventLoopThread::~EventLoopThread() {
    _exiting = true;
    _loop->quit();
    _thread.join();
}

EventLoop* EventLoopThread::startLoop() {
    assert(!_thread.started());
    _thread.start();
    {
        MutexLockGuard lock(_mutex);
        while (_loop == NULL) {
            _cond.wait();
        }
    }

    return _loop;
}

void EventLoopThread::threadFunc() {
    EventLoop loop;
    {
        MutexLockGuard lock(_mutex);
        _loop = &loop;
        _cond.notify();
    }
    
    loop.loop();
}
} // namespace rpc
} // namespace checkking
