#include <string.h>
#include <boost/bind.hpp>
#include <boost/implicit_cast.hpp>
#include <errno.h>

#include "EventLoop.h"
#include "Logging.h"
#include "SocketsOps.h"
#include "TcpConnection.h"

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
        _channel->setReadCallback(boost::bind(&TcpConnection::handleRead, this, _1));
        _channel->setWriteCallback(boost::bind(&TcpConnection::handleWrite, this));
        _channel->setCloseCallback(boost::bind(&TcpConnection::handleClose, this));
        _channel->setErrorCallback(boost::bind(&TcpConnection::handleError, this));
    }

    TcpConnection::~TcpConnection() {
        LOG_TRACE << "TcpConnection::~TcpConnection called:" << _peerAddr.toHostPort();
    }

    void TcpConnection::connectEstablished() {
        _loop->assertInLoopThread();
        assert(_state == CONNECTING);
        setState(CONNECTED);
        _channel->enableReading();

        _connectionCallback(shared_from_this());
    }

    void TcpConnection::handleRead(Timestamp receiveTime) {
        int savedErrno = 0;
        ssize_t n = _inputBuffer.readFd(_channel->fd(), &savedErrno);
        if (n > 0) {
            _messageCallback(shared_from_this(), &_inputBuffer, receiveTime);
        } else if (n == 0) {
            handleClose();
        } else {
            /*
            if (errno == EAGAIN) {
            }
            */
            LOG_ERROR << "Failed to handleRead";
            handleError();
        }
    }

    void TcpConnection::handleWrite() {
        _loop->assertInLoopThread();
        if (_channel->isWriting()) {
            ssize_t n = ::write(_channel->fd(),
                    _outputBuffer.peek(), _outputBuffer.readableBytes());
            if (n > 0) {
                _outputBuffer.retrieve(n);
                if (_outputBuffer.readableBytes() == 0) {
                    _channel->disableWriting();
                   if (_state == DISCONNECTING) {
                       shutdownInLoop();
                   }
                } else {
                    LOG_TRACE << "I am going to write more data";
                }
            } else {
                LOG_ERROR << "An error has happened!";
                _channel->disableWriting();
            }
        } else {
            LOG_TRACE << "Connection is down, no more writing.";
        }
    }

    void TcpConnection::handleClose() {
        _loop->assertInLoopThread();
        LOG_TRACE << "TcpConnection::handleClose state = " << _state;
        assert(_state == CONNECTED || _state == DISCONNECTING);
        _channel->disableAll();
        _closeCallback(shared_from_this());
    }
    
    void TcpConnection::handleError() {
        int err = sockets::getSocketError(_channel->fd());
        LOG_ERROR << "TcpConnection::handleError [" << _name
            << "] - SO_ERROR = " << err << " " << strerror_tl(err);
    }

    void TcpConnection::connectDestroyed() {
        _loop->assertInLoopThread();
        assert(_state == CONNECTED);
        setState(DISCONNECTED);
        _channel->disableAll();
        _connectionCallback(shared_from_this());
        _loop->removeChannel(get_pointer(_channel));
        LOG_INFO << "TcpConnection::connectDestroyed ["
                << _name << "]";
    }

    void TcpConnection::send(const std::string& message) {
        if (_state == CONNECTED) {
            if (_loop->isInLoopThread()) {
                sendInLoop(message);
            } else {
                /// we donnot use share_from_this() (using **this**)
                /// because we are sure the life time of TcpConnection
                /// is longer than boost::bind function.
                _loop->runInLoop(boost::bind(&TcpConnection::sendInLoop, this, message));
            }
        }
    }

    void TcpConnection::send(Buffer* message) {
        if (_state == CONNECTED) {
            if (_loop->isInLoopThread()) {
                sendInLoop(buf->peek(), buf->readableBytes());
                buf->retrieveAll();
            } else {
                _loop->runInLoop(
                        boost::bind(&TcpConnection::sendInLoop,
                            this,
                            buf->retrieveAllAsString()));
            }
        }
    
    }
/*
    void TcpConnection::sendInLoop(const std::string& message) {
        _loop->assertInLoopThread();
        ssize_t nwrote = 0;
        if (!_channel->isWriting() && _outputBuffer.readableBytes() == 0) {
            nwrote = ::write(_channel->fd(), message.data(), message.size());
            if (nwrote >= 0) {
                if (boost::implicit_cast<size_t>(nwrote) < message.size()) {
                    LOG_TRACE << "I am going to write more data.";
                }
            } else {
                nwrote = 0;
                if (errno == EWOULDBLOCK) {
                    LOG_ERROR << "TcpConnection::sendInLoop failed!";
                }
            }
        }
        assert(nwrote >= 0);
        if (boost::implicit_cast<size_t>(nwrote) < message.size()) {
            _outputBuffer.append(message.data() + nwrote, message.size() - nwrote);
        }
        if (_channel->isWriting()) {
            _channel->enableWriting();
        }
    }
 */

    void TcpConnection::shutdown() {
        if (_state == CONNECTED) {
            setState(DISCONNECTING);
            _loop->runInLoop(boost::bind(&TcpConnection::shutdownInLoop, this));
        }
        if (_channel->isWriting()) {
            _socket->shutdownWrite();
        }
    }

    void TcpConnection::shutdownInLoop() {
        _loop->assertInLoopThread();
    }

    void TcpConnection::setTcpNoDelay(bool on) {
        _socket->setTcpNoDelay(on);
    }
} // namespace rpc
} // namespace checkking
