#ifndef CHECKKING_RPC_NET_EVENTLOOP_H
#define CHECKKING_RPC_NET_EVENTLOOP_H

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include "ThreadComm.h"
#include <vector>

namespace checkking {
namespace rpc {

class Channel;
class Poller;

class EventLoop : boost::noncopyable {
public:
    typedef std::vector<Channel*> ChannelList;

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

    static EventLoop* getEventLoopOfCurrentThread();
private:
    void abortNotInLoopThread();
    bool _looping;
    const pid_t _threadId;
    bool _quit;
    boost::scoped_ptr<Poller> _poller;
    ChannelList _activeChannels;
}; // EventLoop

} // namespace rpc
} // namespace checkking
#endif  // CHECKKING_RPC_NET_EVENTLOOP_H
