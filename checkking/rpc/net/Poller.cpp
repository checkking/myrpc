#include "Poller.h"

#include <poll.h>

#include "Timestamp.h"
#include "Channel.h"
#include "Logging.h"

namespace checkking {
namespace rpc {
Poller::Poller(EventLoop* loop): _ownerLoop(loop) {}
Poller::~Poller() {}

Timestamp Poller::poll(int timeoutMs, ChannelList* activeChannels) {
    int numsEvents = ::poll(&*_pollfds.begin(), _pollfds.size(), timeoutMs);
    Timestamp now(Timestamp::now());
    if (numsEvents > 0) {
        fillAcvieChannels(numsEvents, activeChannels);
    } else if (numsEvents == 0) {
        LOG_TRACE << "Nothing happends.";
    } else {
        LOG_ERROR << "::poll failed.";
    }
    return now;
}

void Poller::fillAcvieChannels(int numsEvents, ChannelList* activeChannels) {
    for (PollFdList::const_iterator fdItr = _pollfds.begin(); 
            fdItr != _pollfds.end() && numsEvents > 0; ++fdItr) {
        if (fdItr->revents > 0) {
            --numsEvents;
            ChannelMap::const_iterator chItr = _channels.find(fdItr->fd);
            assert(chItr != _channels.end());
            Channel* channel = chItr->second;
            channel->setRevents(fdItr->revents);
            activeChannels->push_back(channel);
        }
    }
}

void Poller::updateChannel(Channel* channel) {
    assertInLoopThread();
    if (channel->index() < 0) {
        assert(_channels.find(channel->fd()) == _channels.end());
        struct pollfd pfd;
        pfd.fd = channel->fd();
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;
        _pollfds.push_back(pfd);
        int idx = static_cast<int>(_pollfds.size()) - 1;
        _channels[pfd.fd] = channel;
    } else {
        assert(_channels.find(channel->fd()) != _channels.end());
        assert(channel->index() < static_cast<int>(_pollfds.size()));
        struct pollfd& pfd = _pollfds[channel->index()];
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;
    }
}
} // namespace rpc
} // namespace checkking
