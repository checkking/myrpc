#ifndef CHECKKING_RPC_BASE_LOGSTREAM_H
#define CHECKKING_RPC_BASE_LOGSTREAM_H

#include <boost/noncopyable.hpp>
#include <string.h>
#include <string>

namespace checkking {
namespace rpc {

const int gSmallBuffer = 4 * 1024;
const int gLargeBuffer = 1024 * 1024;

template<int SIZE>
class FixedBuffer : boost::noncopyable {
public:
    FixedBuffer() : _cur(_data) {}
    ~FixedBuffer() {}
    void append(const char* buf, size_t len) {
        if (static_cast<size_t>(avial()) > len) {
            memcpy(_cur, buf, len);
            _cur += len;
        }
    }
    const char* data() const {
        return _data;
    }
    int length() const {
        return static_cast<int>(_cur - _data);
    }
    int avial() const {
        return static_cast<int>(end() - _cur);
    }
    void add(size_t len) {
        _cur += len;
    }
    char* current() {
        return _cur;
    }
    void reset() {
        _cur = _data;
    }
    void bzero() {
        ::bzero(_data, sizeof _data);
    }
    std::string str() const {
        return std::string(_data, length());
    }
private:
    const char* end() const {
        return _data + sizeof _data;
    }
    char _data[SIZE];
    char* _cur;
}; // class FixedBuffer

class LogStream : boost::noncopyable {
typedef LogStream self;
public:
typedef FixedBuffer<gSmallBuffer> Buffer;

self& operator<<(bool v) {
    _buffer.append(v ? "1" : "0", 1);
    return *this;
}

self& operator<<(short v);
self& operator<<(unsigned short v);
self& operator<<(int v);
self& operator<<(unsigned int v);
self& operator<<(long v);
self& operator<<(unsigned long v);
self& operator<<(long long v);
self& operator<<(unsigned long long v);

self& operator<<(const void* v);
self& operator<<(float v) {
    *this << (static_cast<double>(v));
    return *this;
}
self& operator<<(double v);
self& operator<<(char v) {
    _buffer.append(&v, 1);
    return *this;
}

self& operator<<(const char* v) {
    if (v) {
        _buffer.append(v, strlen(v));
    } else {
        _buffer.append("(null)", 6);
    }

    return *this;
}

self& operator<<(const std::string& v) {
    _buffer.append(v.c_str(), v.size());

    return *this;
}

void append(const char* data, int len) {
    _buffer.append(data, len);
}

void resetBuffer() {
    _buffer.reset();
}

const Buffer& buffer() const {
    return _buffer;
}

private:
    Buffer _buffer;
    template<typename T>
    void formatInteger(T);

    static const int _s_max_numeric_size = 32;
}; // class LogStream

class Fmt {
public:
    template<typename T>
    Fmt(const char* fmt, T val);
    const char* data() const {
        return _buf;
    }
    int length() const {
        return _length;
    }

private:
    char _buf[32];
    int _length;
}; // class Fmt

} // namespace rpc
} // namespace checkking

#endif // CHECKKING_RPC_BASE_LOGSTREAM_H
