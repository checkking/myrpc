#include "Channel.h"
#include "EventLoop.h"

#include <poll.h>

namespace checkking {
namespace rpc {

Channel::Channel(EventLoop* loop, int fd): _loop(loop), _fd(fd),
        _events(0), _revent(0), _index(-1){}

void Channel::update() {
    _loop->updateChannel(this);
}

void Channel::handleEvent() {
    if (_revent & (POLLERR | POLLNVAL)) {
        if (_errorCallback) {
            _errorCallback();
        }
    }
    if (_revent & (POLLIN | POLLPRI | POLLRDHUP)) {
        if (_readCallback) {
            _readCallback();
        }
    }
    if (_revent & POLLOUT) {
        if (_writeCallback) {
            _writeCallback();
        }
    }
}

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;
} // namespace rpc
} // namespace checkking
