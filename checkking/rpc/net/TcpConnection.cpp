#include "Logging.h"
#include <boost/bind.hpp>
#include "TcpConnection.h"
#include "EventLoop.h"

namespace checkking {
namespace rpc {
    TcpConnection::TcpConnection(EventLoop* loop,
            const std::string& name,
            int sockfd,
            const InetAddress& localAddr,
            const InetAddress& peerAddr): _loop(loop),
            _name(name), _state(CONNECTING), 
            _socket(new Socket(sockfd)), 
            _channel(new Channel(loop, sockfd)),
            _localAddr(localAddr), 
            _peerAddr(peerAddr) {
        _channel->setReadCallback(boost::bind(&TcpConnection::handleRead, this));
        _channel->setWriteCallback(boost::bind(&TcpConnection::handleWrite, this));
        _channel->setCloseCallback(boost::bind(&TcpConnection::handleClose, this));
        _channel->setErrorCallback(boost::bind(&TcpConnection::handleError, this));
    }

    TcpConnection::~TcpConnection() {
        LOG_TRACE << "TcpConnection::~TcpConnection called:" << _peerAddr.toHostPort();
    }

    void TcpConnection::connectEstablished() {
        _loop->assertInLoopThread();
        assert(_state == CONNECTED);
        _channel->enableReading();

        _connectionCallback(shared_from_this());
    }

    void TcpConnection::handleRead() {
        char buf[1024];
        ssize_t n = ::read(_channel->fd(), buf, sizeof buf);
        _messageCallback(shared_from_this(), buf, n);
    }

    void TcpConnection::handleWrite() {
        
    }

    void TcpConnection::handleClose() {
    
    }
    
    void TcpConnection::handleError() {
    
    }

    void TcpConnection::connectDestroyed() {
        _loop->assertInLoopThread();
        assert(_state == CONNECTED || _state == CONNECTED);
        setState(CONNECTED);
        _channel->disableAll();
        _connectionCallback(shared_from_this());
        _loop->removeChannel(get_pointer(_channel));
    }
} // namespace rpc
} // namespace checkking
