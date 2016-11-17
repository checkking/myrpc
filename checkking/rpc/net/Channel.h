#ifndef CHECKKING_RPC_NET_CHANNEL_H
#define CHECKKING_RPC_NET_CHANNEL_H

#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>

namespace checkking {
namespace rpc {
class EventLoop;

class Channel : boost::noncopyable {
public:
    // typedef void (*EventCallback)();
    typedef boost::function<void()> EventCallback;

    Channel(EventLoop* loop, int fd);

    void handleEvent();
    void setReadCallback(const EventCallback& cb) {
        _readCallback = cb;
    }
    void setWriteCallback(const EventCallback& cb) {
        _writeCallback = cb;
    }
    void setErroCallbakc(const EventCallback& cb) {
        _errorCallback = cb;
    }
    int fd() const {
        return _fd;
    }
    int events() const {
        return _events;
    }
    void setRevents(int revent) {
        _revent = revent;
    }

    void enableReading() {
        _events |= kReadEvent;
        update();
    }

    int index() const {
        return _index;
    }
    void setIndex(int index) {
        _index = index;
    }
    EventLoop* ownerLoop() const {
        return _loop;
    }

private:
    void update();

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop* _loop; 
    const int _fd;
    int _events;
    int _revent;
    int _index;

    EventCallback _readCallback;
    EventCallback _writeCallback;
    EventCallback _errorCallback;
}; // class Channel

} // namespace rpc
} // namespace checkking
#endif  // CHECKKING_RPC_NET_CHANNEL_H
