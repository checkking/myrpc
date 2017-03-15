#ifndef CHECKKING_RPC_NET_CONNECTOR_H
#define CHECKKING_RPC_NET_CONNECTOR_H

#include "InetAddress.h"
#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

namespace checkking {
namespace rpc {

class EventLoop;
class Channel;

class Connector : boost::noncopyable,
        public boost::enable_shared_from_this<Connector> {
public:
    typedef boost::function<void (int sockfd)> NewConnectionCallback;
    Connector(EventLoop* loop, const InetAddress& serverAddr);
    ~Connector();

    void setNewConnectionCallback(const NewConnectionCallback& cb) {
    }

    void start();  // can be called in any thread
    void restart();  // must be called in loop thread
    void stop();  // can be called in any thread
    const InetAddress& serverAddress() const {
        return _serverAddr;
    }

private:
    enum States { CONNECTING, CONNECTED, DISCONNECTING, DISCONNECTED};
    static const int kMaxRetryDelayMs = 30 * 1000;
    static const int kInitRetryDelayMs = 500;

    void setState(States s) {
        _state = s;
    }

    void startInLoop();
    void stopInLoop();
    void connect();
    void connecting(int sockfd);
    void handleWrite();
    void handleError();
    void retry(int sockfd);
    int removeAndResetChannel();
    void resetChannel();

    EventLoop* _loop;
    InetAddress _serverAddr;
    bool _connected;
    States _state;
    boost::scoped_ptr<Channel> _channel;
    NewConnectionCallback _newConnectionCallback;
    int _retryDelayMs;
}; // class Connector

} // namespace rpc
} // namespace checkking
#endif  // CHECKKING_RPC_NET_CONNECTOR_H
