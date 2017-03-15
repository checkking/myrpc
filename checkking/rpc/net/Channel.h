#ifndef CHECKKING_RPC_NET_CHANNEL_H
#define CHECKKING_RPC_NET_CHANNEL_H

#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include "Timestamp.h"

namespace checkking {
namespace rpc {
class EventLoop;

class Channel : boost::noncopyable {
public:
    // typedef void (*EventCallback)();
    typedef boost::function<void(Timestamp)> ReadEventCallback;
    typedef boost::function<void()> EventCallback;

    Channel(EventLoop* loop, int fd);

    void handleEvent(Timestamp receiveTime);
    void setReadCallback(const ReadEventCallback& cb) {
        _readCallback = cb;
    }
    void setWriteCallback(const EventCallback& cb) {
        _writeCallback = cb;
    }
    void setErrorCallback(const EventCallback& cb) {
        _errorCallback = cb;
    }

    void setCloseCallback(const EventCallback& cb) {
        _closeCallback = cb;
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
    bool isNoneEvent() const { 
        return _events == kNoneEvent;
    }
    void enableWriting() {
        _events |= kWriteEvent; 
        update();
    }

    void enableReading() {
        _events |= kReadEvent;
        update();
    }

    void disableWriting() {
        _events &= ~kWriteEvent;
        update();
    }

    void disableAll() {
        _events = kNoneEvent;
        update();
    }

    bool isWriting() const {
        return _events & kWriteEvent;
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
    void remove();

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

    ReadEventCallback _readCallback;
    EventCallback _writeCallback;
    EventCallback _errorCallback;
    EventCallback _closeCallback;
}; // class Channel

} // namespace rpc
} // namespace checkking
#endif  // CHECKKING_RPC_NET_CHANNEL_H
