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
        if (n > 0) {
            _messageCallback(shared_from_this(), buf, n);
        } else if (n == 0) {
            handleClose();
        } else {
            LOG_ERROR << "Failed to handleRead";
            handleError();
        }
    }

    void TcpConnection::handleWrite() {
        _loop->assertInLoopThread();
        if (channel_->isWriting()) {
            ssize_t n = ::write(channel_->fd(),
                    _outputBuf, strlen(_outputBuf));
            if (n == 0 || n == strlen(_outputBuf)) {
                _channel->disableWriting();
            } else if (n > 0) {
                LOG_WARN << "Write not finished.";
                _channel->disableWriting();
            } else {
                LOG_TRACE << "An error has happened!";
                _channel->disableWriting();
            }
        }
    }

    void TcpConnection::handleClose() {
    
    }
    
    void TcpConnection::handleError() {
    
    }

    void TcpConnection::connectDestroyed() {
        _loop->assertInLoopThread();
        assert(_state == CONNECTED);
        setState(DISCONNECTED);
        _channel->disableAll();
        _connectionCallback(shared_from_this());
        _loop->removeChannel(get_pointer(_channel));
    }

    void TcpConnection::send(const std::string& message) {
        _loop->assertInLoopThread();
        assert(_state == CONNECTED);
        _outputBuf = message;
        _channel->enableWriting();
    }
} // namespace rpc
} // namespace checkking
