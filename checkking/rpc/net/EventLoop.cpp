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
        _poller->poll(kPollTimeMs, &_activeChannels);
        for (ChannelList::iterator itr = _activeChannels.begin(); 
                itr != _activeChannels.end(); ++itr) {
            (*itr)->handleEvent();
        }
    }
    LOG_TRACE << "EventLoop " << this << " stop looping";
    _looping = false;
}

void EventLoop::updateChannel(Channel* channel) {
    _poller->updateChannel(channel);
}
} // namespace rpc
} // namespace checkking
