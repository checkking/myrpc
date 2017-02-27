#ifndef CHECKKING_RPC_NET_TCPCONNECTION_H
#define CHECKKING_RPC_NET_TCPCONNECTION_H

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <string>
#include "InetAddress.h"
#include "Callbacks.h"
#include "Socket.h"
#include "Channel.h"

namespace checkking {
namespace rpc {
class EventLoop;
class TcpConnection : boost::noncopyable, public boost::enable_shared_from_this<TcpConnection> {
public:
    TcpConnection(EventLoop* loop,
            const std::string& name, 
            int sockfd,
            const InetAddress& localAddr,
            const InetAddress& peerAddr);
    ~TcpConnection();

    EventLoop* getLoop() const {
        return _loop;
    }

    const std::string& name() const {
        return _name;
    }

    const InetAddress& localAddress() const {
        return _localAddr;
    }

    const InetAddress& peerAddress() const {
        return _peerAddr;
    }

    bool connected() const {
        return _state == CONNECTED;
    }

    void connectEstablished();

    void setConnectionCallback(const ConnectionCallback& cb) {
        _connectionCallback = cb;
    }

    void setMessageCallback(const MessageCallback& cb) {
        _messageCallback = cb;
    }
    void setCloseCallback(const CloseCallback& cb) {
        _closeCallback = cb;
    }
    void connectDestroyed();

    void send(const std::string& message);

private:
    enum StateE {
        CONNECTING,
        CONNECTED,
        DISCONNECTING,
        DISCONNECTED
    };
    void setState(StateE s) {
        _state = s;
    }
    void handleRead();
    void handleWrite();
    void handleClose();
    void handleError();

    EventLoop* _loop;
    std::string _name;
    StateE _state;
    boost::scoped_ptr<Socket> _socket;
    boost::scoped_ptr<Channel> _channel;
    InetAddress _localAddr;
    InetAddress _peerAddr;
    ConnectionCallback _connectionCallback;
    MessageCallback _messageCallback;
    CloseCallback _closeCallback;
    char _outputBuf[1024];
    char _inputBuf[1024];
}; // class TcpConnection

} // namespace rpc
} // namespace checkking
#endif  // CHECKKING_RPC_NET_TCPCONNECTION_H
