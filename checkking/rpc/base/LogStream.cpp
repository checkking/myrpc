#include "LogStream.h"
#include <string.h>
#include <algorithm>
#include <stdio.h>
#include <stdint.h>

namespace checkking {
namespace rpc {
const char digits[] = "9876543210123456789";
const char* zero = digits + 9;
const char digitsHex[] = "0123456789ABCDEF";
template<typename T>
size_t convert(char buf[], T value) {
    T i = value;
    char* p = buf;
    do {
        int lsd = static_cast<int>(i % 10);
        i /= 10;
        *p++ = zero[lsd];
    } while (i != 0);
    if (value < 0) {
        *p++ = '-';
    }
    *p = '\0';
    std::reverse(buf, p);

    return p - buf;
}

size_t convertHex(char buf[], uintptr_t value) {
    uintptr_t i = value;
    char *p = buf;
    do {
       int lsd = static_cast<int>(i % 16);
       i /= 16;
       *p++ = digitsHex[lsd];
    } while (i != 0);
    *p = '\0';
    std::reverse(buf, p);

    return p - buf;
}

template class FixedBuffer<gSmallBuffer>;
template class FixedBuffer<gLargeBuffer>;

template<typename T>
void LogStream::formatInteger(T v) {
    if (_buffer.avial() > _s_max_numeric_size) {
        size_t len = convert(_buffer.current(), v);
        _buffer.add(len);
    }
}

LogStream& LogStream::operator<<(short v) {
    *this << static_cast<int>(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned short v) {
    *this << static_cast<int>(v);
    return *this;
}

LogStream& LogStream::operator<<(int v) {
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned int v) {
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(long v) {
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned long v) {
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(long long v) {
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned long long v) {
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(const void* p) {
    uintptr_t v = reinterpret_cast<uintptr_t>(p);
    if (_buffer.avial() >= _s_max_numeric_size) {
        char* buf = _buffer.current();
        buf[0] = '0';
        buf[1] = 'x';
        size_t len = convertHex(buf + 2, v);
        _buffer.add(len + 2);
    }

    return *this;
}

LogStream& LogStream::operator<<(double v) {
    if (_buffer.avial() >= _s_max_numeric_size) {
        int len = snprintf(_buffer.current(), _s_max_numeric_size, "%.12g", v);
        _buffer.add(len);
    }

    return *this;
}

template<typename T>
Fmt::Fmt(const char* fmt, T val) {
    _length = snprintf(_buf, sizeof _buf, fmt, val);
}

template Fmt::Fmt(const char* fmt, char);
template Fmt::Fmt(const char* fmt, short);
template Fmt::Fmt(const char* fmt, unsigned short);
template Fmt::Fmt(const char* fmt, int);
template Fmt::Fmt(const char* fmt, unsigned int);
template Fmt::Fmt(const char* fmt, long);
template Fmt::Fmt(const char* fmt, unsigned long);
template Fmt::Fmt(const char* fmt, long long);
template Fmt::Fmt(const char* fmt, unsigned long long);
template Fmt::Fmt(const char* fmt, float);
template Fmt::Fmt(const char* fmt, double);
} // namespace rpc
} // namespace checkking
