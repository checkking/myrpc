#include "EventLoop.h"

#include <poll.h>
#include <assert.h>
#include "Logging.h"
#include "Poller.h"
#include "Channel.h"

namespace checkking {
namespace rpc {

__thread EventLoop* t_loopInThisThread = NULL;
const int kPollTimeMs = 10000;

EventLoop::EventLoop():_looping(false), _threadId(CurrentThread::tid()),
        _quit(false), _poller(new Poller(this)) {
    if (t_loopInThisThread) {
        LOG_FATAL << "Another EventLoop already in this thread " << _threadId;
    } else {
        t_loopInThisThread = this;
    }
}

void EventLoop::runInLoop(const Functor& cb) {
    if (isInLoopThread()) {
        cb();
    } else {
        queueInLoop(cb);
    }
}

void EventLoop::queueInLoop(const Functor& cb) {
    {
        MutexLockGuard lock(_mutex); 
        _pendingFunctors.push_back(cb);
    }
    if (!isInLoopThread() || _callingPendingFunctors) {
        wakeup();
    }
}

void EventLoop::wakeup() {
    uint64_t one = 1;
    ssize_t n = ::write(_wakeupFd, &one, sizeof one);
    if (n != sizeof one) {
        LOG_ERROR << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
    }
}

EventLoop::~EventLoop() {
    assert(!_looping);
    t_loopInThisThread = NULL;
}

EventLoop* EventLoop::getEventLoopOfCurrentThread() {
    return t_loopInThisThread;
}

void EventLoop::abortNotInLoopThread() {
    LOG_FATAL << "EventLoop " << this << " was created in thread:" << _threadId
            << ", but current thread is:" << CurrentThread::tid();
}

void EventLoop::loop() {
    assert(!_looping);
    assertInLoopThread();
    _looping = true;
    while (!_quit) {
        _activeChannels.clear();
        Timestamp receiveTime = _poller->poll(kPollTimeMs, &_activeChannels);
        for (ChannelList::iterator itr = _activeChannels.begin(); 
                itr != _activeChannels.end(); ++itr) {
            (*itr)->handleEvent(receiveTime);
        }
        doPendingFunctors();
    }
    LOG_TRACE << "EventLoop " << this << " stop looping";
    _looping = false;
}

void EventLoop::updateChannel(Channel* channel) {
    _poller->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel) {
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    _poller->removeChannel(channel);
}

void EventLoop::doPendingFunctors() {
    std::vector<Functor> functors;
    _callingPendingFunctors = true;
    {
        MutexLockGuard lock(_mutex);
        functors.swap(_pendingFunctors);
    }
    for (size_t i = 0; i < functors.size(); ++i) {
        functors[i]();
    }
    _callingPendingFunctors = false;
}
} // namespace rpc
} // namespace checkking
