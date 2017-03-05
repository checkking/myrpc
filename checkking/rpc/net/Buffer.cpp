#include "Buffer.h"

#include "Buffer.h"
#include "SocketsOps.h"
#include "logging/Logging.h"

#include <errno.h>
#include <memory.h>
#include <sys/uio.h>

namespace checkking {
namespace rpc {
ssize_t Buffer::readFd(int fd, int* savedErrno) {
    char extrabuf[65536];
    struct iovec vec[2];
    const size_t writable = writableBytes();
    vec[0].iov_base = begin() + _writerIndex;
    vec[0].iov_len = writable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof extrabuf;
    const ssize_t n = readv(fd, vec, 2);
    if (n < 0) {
        *savedErrno = errno;
    } else if (implicit_cast<size_t>(n) <= writable) {
        _writerIndex += n;
    } else {
        _writerIndex = _buffer.size();
        append(extrabuf, n - writable);
    }

    return n;
}
} // namespace rpc
} // namespace checkking
