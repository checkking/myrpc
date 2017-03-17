#ifndef CHECKKING_RPC_NET_EPOLLER_H
#define CHECKKING_RPC_NET_EPOLLER_H

#include <map>
#include <vector>
#include "Timestamp.h"
#include "EventLoop.h"

struct epoll_event;

namespace checkking {
namespace rpc {
class Channel;

class EPoller : boost::noncopyable {
public:
    typedef std::vector<Channel*> ChannelList;
    EPoller(EventLoop* loop);
    ~EPoller();
    Timestamp poll(int timeoutMs, ChannelList* activeChannels);

    void updateChannel(Channel* channel);

    void removeChannel(Channel* channel);

    void assertInLoopThread() {
        _ownerLopp->assertInLoopThread();
    }

private:
    static const int kInitEventListSize = 16;

    void fillActiveChannels(int numEvents,
            ChannelList* activeChannels) const;

    void update(int operation, Channel* channel);

    typedef std::vector<struct epoll_event> EventList;
    typedef std::map<int, Channel*> ChannelMap;

    EventLoop* _ownerLoop;
    int _epollfd;
    EventList _events;
    ChannelMap _channels;
}; // class EPoller
} // namespace rpc
} // namespace checkking
#endif  // CHECKKING_RPC_NET_EPOLLER_H
