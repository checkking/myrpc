#include <assert.h>
#include <errno.h>
#include <poll.h>
#include <sys/epoll.h>
#include <boost/static_assert.hpp>
#include <boost/implicit_cast.hpp>

#include "Channel.h"
#include "Logging.h"
#include "EPoller.h"

BOOST_STATIC_ASSERT(EPOLLIN == POLLIN);
BOOST_STATIC_ASSERT(EPOLLPRI == POLLPRI);
BOOST_STATIC_ASSERT(EPOLLOUT == POLLOUT);
BOOST_STATIC_ASSERT(EPOLLRDHUP == POLLRDHUP);
BOOST_STATIC_ASSERT(EPOLLERR == POLLERR);
BOOST_STATIC_ASSERT(EPOLLHUP == POLLHUP);

namespace checkking {
namespace rpc {

const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;

EPoller::EPoller(EventLoop* loop)
    : _ownerLoop(loop),
    _epollfd(::epoll_create1(EPOLL_CLOEXEC)),
    _events(kInitEventListSize) {
    if (_epollfd < 0) {
        LOG_ERROR << "EPoller::EPoller";
    }
}

EPoller::~EPoller() {
    ::close(_epollfd);
}

Timestamp EPoller::poll(int timeoutMs, ChannelList* activeChannels) {
    int numEvents = ::epoll_wait(_epollfd,
                                &*_events.begin(),
                                static_cast<int>(_events.size()),
                                timeoutMs);
    Timestamp now(Timestamp::now());
    if (numEvents > 0) {
        LOG_TRACE << numEvents << " events happended";
        fillActiveChannels(numEvents, activeChannels);
    } else if (numEvents == 0) {
        LOG_TRACE << "nothing happened.";
    } else {
        LOG_ERROR << "EPoller::poll()";
    }

    return now;
}

void EPoller::updateChannel(Channel* channel) {
    assertInLoopThread();
    LOG_TRACE << "fd=" << channel->fd() << " events="
            << channel->events();
    const int index = channel->index();
    if (index == kNew || index == kDeleted) {
        int fd = channel->fd();
        if (index == kNew) {
            assert(_channels.find(fd) == _channels.end());
            _channels[fd] = channel;
        } else {
            assert(_channels.find(fd) != _channels.end());
            assert(_channels[fd] == channel);
        }
        channel->setIndex(kAdded);
        update(EPOLL_CTL_ADD, channel);
    } else {
        int fd = channel->fd();
        (void)fd;
        assert(_channels.find(fd) != _channels.end());
        assert(_channels[fd] == channel);
        assert(index == kAdded);
        if (channel->isNoneEvent()) {
            update(EPOLL_CTL_DEL, channel);
        } else {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EPoller::removeChannel(Channel* channel) {
    assertInLoopThread();
    int fd = channel->fd();
    LOG_TRACE << "fd = " << fd;
    assert(_channels.find(fd) != _channels.end());
    assert(_channels[fd] == channel);
    assert(channel->isNoneEvent());
    int index = channel->index();
    assert(index == kAdded || index == kDeleted);
    size_t n = _channels.erase(fd);
    (void)n;
    assert(n == 1);
    if (index == kAdded) {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->setIndex(kNew);
}

void EPoller::update(int operation, Channel* channel) {
    struct epoll_event event;
    bzero(&event, sizeof event);
    event.events = channel->events();
    event.data.ptr = channel;
    int fd = channel->fd();
    if (::epoll_ctl(_epollfd, operation, fd, &event) < 0) {
        if (operation == EPOLL_CTL_DEL) {
            LOG_ERROR << "epoll_ctl op=" << operation << " fd=" << fd;
        } else {
            LOG_FATAL << "epoll_ctl op=" << operation << " fd=" << fd;
        }
    }
}

void EPoller::fillActiveChannels(int numEvents,
                                ChannelList* activeChannels) const {
    assert(boost::implicit_cast<size_t>(numEvents) <= _events.size());
    for (int i = 0; i < numEvents; ++i) {
        Channel* channel = static_cast<Channel*>(_events[i].data.ptr);
        channel->setRevents(_events[i].events);
        activeChannels->push_back(channel);
    }
}
} // namespace rpc
} // namespace checkking
