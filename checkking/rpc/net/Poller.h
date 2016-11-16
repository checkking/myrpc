#ifndef CHECKKING_RPC_NET_POLLER_H
#define CHECKKING_RPC_NET_POLLER_H

#include <vector>
#include <map>
#include <boost/noncopyable.hpp>
#include "EventLoop.h"

struct pollfd;

namespace checkking {
namespace rpc {
class Channel;
class Timestamp;

class Poller : boost::noncopyable {
public:
    typedef std::vector<Channel*> ChannelList;

    Poller(EventLoop* loop);
    ~Poller();

    Timestamp poll(int timeoutMs, ChannelList* activeChannels);

    void updateChannel(Channel* channel);
    void assertInLoopThread() {
        _ownerLoop->assertInLoopThread();
    }

private:
    typedef std::vector<struct pollfd> PollFdList;
    typedef std::map<int, Channel*> ChannelMap;

    void fillAcvieChannels(int numsEvents, ChannelList* activeChannels);

    EventLoop* _ownerLoop;
    PollFdList _pollfds;
    ChannelMap _channels;

}; // class Poller

} // namespace rpc
} // namespace checkking
#endif  // CHECKKING_RPC_NET_POLLER_H
