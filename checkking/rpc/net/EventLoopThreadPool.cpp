#include "EventLoopThreadPool.h"

#include "EventLoop.h"
#include "EventLoopThread.h"

#include <boost/bind.hpp>

namespace checkking {
namespace rpc {
void EventLoopThreadPool::start() {
    assert(!_started);
    _baseLoop->assertInLoopThread();
    _started = true;

    for (int i = 0; i < _numThreads; ++i) {
        EventLoopThread* t = new EventLoopThread();
        _threads.push_back(t);
        _loops.push_back(t->startLoop());
    }
}

EventLoop* EventLoopThreadPool::getNextLoop() {
    _baseLoop->assertInLoopThread();
    EventLoop* loop = _baseLoop;
    if (!_loops.empty()) {
        // round-robin
        loop = _loops[_next++];
        if (static_cast<size_t>(_next) >= _loops.size()) {
            _next = 0;
        }
    }
    return loop;
}
} // namespace rpc
} // namespace checkking
