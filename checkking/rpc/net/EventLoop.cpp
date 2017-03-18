#include "EventLoop.h"

#include <poll.h>
#include <assert.h>
#include <signal.h>
#include <sys/eventfd.h>
#include "Logging.h"
// #include "Poller.h"
#include "EPoller.h"
#include "Channel.h"
#include "TimerQueue.h"

namespace checkking {
namespace rpc {

__thread EventLoop* t_loopInThisThread = NULL;
const int kPollTimeMs = 10000;

class IgnoreSigPipe {
public:
    IgnoreSigPipe() {
        ::signal(SIGPIPE, SIG_IGN);
    }
};

IgnoreSigPipe initObj;

int createEventfd()
{
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0) {
        LOG_ERROR << "Failed in eventfd";
        abort();
    }
    return evtfd;
}

EventLoop::EventLoop()
    :_looping(false),
    _threadId(CurrentThread::tid()),
    _quit(false),
    _poller(new EPoller(this)),
    _activeChannels(),
    _mutex(),
    _pendingFunctors(),
    _wakeupFd(createEventfd()),
    _timerQueue(new TimerQueue(this)) {
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

TimerId EventLoop::runAt(const Timestamp& time, const TimerCallback& cb) {
    return _timerQueue->addTimer(cb, time, 0.0);
}

TimerId EventLoop::runAfter(double delay, const TimerCallback& cb)
{
    Timestamp time(addTime(Timestamp::now(), delay));
    return runAt(time, cb);
}

TimerId EventLoop::runEvery(double interval, const TimerCallback& cb)
{
    Timestamp time(addTime(Timestamp::now(), interval));
    return _timerQueue->addTimer(cb, time, interval);
}
} // namespace rpc
} // namespace checkking
