#ifndef CHECKKING_RPC_NET_EVENTLOOP_H
#define CHECKKING_RPC_NET_EVENTLOOP_H

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/function.hpp>
#include "Mutex.h"
#include "ThreadComm.h"
#include <vector>

namespace checkking {
namespace rpc {

class Channel;
class Poller;

class EventLoop : boost::noncopyable {
public:
    typedef std::vector<Channel*> ChannelList;
    typedef boost::function<void()> Functor;

    EventLoop();
    ~EventLoop();

    void loop();
    void assertInLoopThread() {
        if (!isInLoopThread()) {
            abortNotInLoopThread();
        }
    }

    bool isInLoopThread() const {
        return _threadId == CurrentThread::tid();
    }

    void updateChannel(Channel* channel);

    void quit() {
        _quit = true;    
    }

    void runInLoop(const Functor& cb);

    void queueInLoop(const Functor& cb);

    void wakeup();

    void removeChannel(Channel* channel);

    static EventLoop* getEventLoopOfCurrentThread();
private:
    void abortNotInLoopThread();
    void doPendingFunctors();

    bool _looping;
    const pid_t _threadId;
    bool _quit;
    boost::scoped_ptr<Poller> _poller;
    ChannelList _activeChannels;
    MutexLock _mutex;
    std::vector<Functor> _pendingFunctors;
    bool _callingPendingFunctors;
    int _wakeupFd;
}; // EventLoop

} // namespace rpc
} // namespace checkking
#endif  // CHECKKING_RPC_NET_EVENTLOOP_H
