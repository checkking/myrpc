#include "Connector.h"
#include "Logging.h"
#include "Channel.h"
#include "EventLoop.h"
#include "SocketsOps.h"

#include <boost/bind.hpp>
#include <errno.h>

namespace checkking {
namespace rpc {
Connector::Connector(EventLoop* loop, const InetAddress& serverAddr):
    _loop(loop),
    _serverAddr(serverAddr), 
    _connected(false),
    _state(DISCONNECTED), 
    _retryDelayMs(kInitRetryDelayMs) {}

Connector::~Connector() {
    assert(!_channel);
}

void Connector::start() {
    _connected =  true;
    _loop->runInLoop(boost::bind(&Connector::startInLoop, this));
}

void Connector::startInLoop() {
    _loop->assertInLoopThread();
    assert(_state == DISCONNECTED);
    if (_connected) {
        connect();
    }
}

void Connector::stop() {
    _connected = false;
    _loop->queueInLoop(boost::bind(&Connector::stopInLoop, this));
}

void Connector::stopInLoop() {
    _loop->assertInLoopThread();
    if (_state == CONNECTING) {
        setState(DISCONNECTED);
        int sockfd = removeAndResetChannel();
        retry(sockfd);
    }
}

void Connector::connect() {
    int sockfd = sockets::createNonblockingOrDie(_serverAddr.family());
    int ret = sockets::connect(sockfd, _serverAddr.getSockAddr());
    int savedErrno = (ret == 0) ? 0 : errno;
    switch (savedErrno) {
    case 0:
    case EINPROGRESS:
    case EINTR:
    case EISCONN:
        connecting(sockfd);
        break;
    case EAGAIN:
    case EADDRINUSE:
    case EADDRNOTAVAIL:
    case ECONNREFUSED:
    case ENETUNREACH:
        retry(sockfd);
        break;
    case EACCES:
    case EPERM:
    case EAFNOSUPPORT:
    case EALREADY:
    case EBADF:
    case EFAULT:
    case ENOTSOCK:
        LOG_ERROR << "connect error in Connector::startInLoop " << savedErrno;
        sockets::close(sockfd);
        break;
    default:
        LOG_ERROR << "Unexpected error in Connector::startInLoop " << savedErrno;
        sockets::close(sockfd);
        break;
    }
}

void Connector::restart() {
    _loop->assertInLoopThread();
    setState(DISCONNECTED);
    _retryDelayMs = kInitRetryDelayMs;
    _connected = true;
    startInLoop();
}

void Connector::connecting(int sockfd) {
    setState(CONNECTING);
    assert(!_channel);
    _channel.reset(new Channel(_loop, sockfd));
    _channel->setWriteCallback(boost::bind(&Connector::handleWrite, this));
    _channel->setErrorCallback(boost::bind(&Connector::handleError, this));
    _channel->enableWriting();
}

int Connector::removeAndResetChannel() {
    _channel->disableAll();
    _channel->remove();
    int sockfd = _channel->fd();
    _loop->queueInLoop(boost::bind(&Connector::resetChannel, this));
    return sockfd;
}

void Connector::resetChannel() {
    _channel.reset();
}

void Connector::handleWrite() {
    if (_state == CONNECTING) {
        int sockfd = removeAndResetChannel();
        int err = sockets::getSocketError(sockfd);
        if (err) {
            LOG_WARN << "Connector::handleWrite - SO_ERROR = "
                << err << " " << strerror_tl(err);
            retry(sockfd);
        } else if (sockets::isSelfConnect(sockfd)) {
            LOG_WARN << "Connector::handleWrite - Self connect";
            retry(sockfd);
        } else {
             setState(CONNECTED);
             if (_connected) {
                _newConnectionCallback(sockfd);
             } else {
                 sockets::close(sockfd);
             }
        }
    } else {
        assert(_state == DISCONNECTED);
    }
}

void Connector::handleError() {
    LOG_ERROR << "Connector::handleError state=" << _state;
    if (_state == CONNECTING) {
        int sockfd = removeAndResetChannel();
        int err = sockets::getSocketError(sockfd);
        LOG_TRACE << "SO_ERROR = " << err << " " << strerror_tl(err);
        retry(sockfd);
    }
}

void Connector::retry(int sockfd) {
    sockets::close(sockfd);
    setState(DISCONNECTED);
    if (_connected) {
        LOG_INFO << "Connector::retry - Retry connecting to "
            << _serverAddr.toIpPort()
            << " in " << _retryDelayMs
            << " milliseconds.";
        _loop->runAfter(_retryDelayMs / 1000.0,
                boost::bind(&Connector::startInLoop, shared_from_this()));
    } else {
        LOG_DEBUG << "do not connect";
    }
}
} // namespace rp
} // namespace checkking
