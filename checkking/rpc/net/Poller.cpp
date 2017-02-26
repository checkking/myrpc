#include "Poller.h"
#include <poll.h>
#include <boost/implicit_cast.hpp>
#include <algorithm>

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

void Poller::removeChannel(Channel* channel) {
    assertInLoopThread();
    LOG_TRACE << "fd = " << channel->fd();
    assert(_channels.find(channel->fd()) != _channels.end());
    assert(_channels[channel->fd()] == channel);
    assert(channel->isNoneEvent());
    int idx = channel->index();
    assert(0 <= idx && idx < static_cast<int>(_pollfds.size()));
    const struct pollfd& pfd = _pollfds[idx]; (void)pfd;
    assert(pfd.fd == -channel->fd()-1 && pfd.events == channel->events());
    size_t n = _channels.erase(channel->fd());
    assert(n == 1); (void)n;
    if (boost::implicit_cast<size_t>(idx) == _pollfds.size()-1) {
        _pollfds.pop_back();
    } else {
        int channelAtEnd = _pollfds.back().fd;
        iter_swap(_pollfds.begin()+idx, _pollfds.end()-1);
        if (channelAtEnd < 0) {
            channelAtEnd = -channelAtEnd-1;
        }
        _channels[channelAtEnd]->setIndex(idx);
        _pollfds.pop_back();
    }
}
} // namespace rpc
} // namespace checkking
