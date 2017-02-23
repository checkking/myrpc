#ifndef CHECKKING_RPC_NET_ACCEPTOR_H
#define CHECKKING_RPC_NET_ACCEPTOR_H

#include <boost/noncopyable.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "Channel.h"
#include "Socket.h"

namespace checkking {
namespace rpc {

class  EventLoop;
class InetAddress;

class Acceptor : boost::noncopyable {
public:
    // typedef void (*NewConnectionCallback)(int sockfd, const InetAddress&);
    typedef boost::function<void (int sockfd,
            const InetAddress&)> NewConnectionCallback;
    Acceptor(EventLoop* loop, const InetAddress& listenAddr);

    void setNewConnectionCallback(const NewConnectionCallback& cb) {
        _newConnectionCallback = cb;
    }

    bool listenning() const {
        return _listenning;
    }

    void listen();

private:
    void handleRead();

    EventLoop* _loop;
    Socket _acceptSocket;
    Channel _acceptChannel;
    NewConnectionCallback _newConnectionCallback;
    bool _listenning;
}; //  class Acceptor

} // namespace rpc
} // namespace checkking
#endif  // CHECKKING_RPC_NET_ACCEPTOR_H
