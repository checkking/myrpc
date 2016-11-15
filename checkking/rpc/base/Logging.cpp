#include "Logging.h"
#include <pthread.h>
#include <stdio.h>
#include "ThreadComm.h"

namespace checkking {
namespace rpc {
__thread char t_time[32];
__thread time_t t_lastSecond;

Logger::LogLevel initLogLevel() {
    if (::getenv("RPC_LOG_TRACE")) {
        return Logger::TRACE;
    } else {
        return Logger::DEBUG;
    }
}

Logger::LogLevel g_loglevel = initLogLevel();

const char* LogLevelName[Logger::NUM_LOG_LEVELS] = {
    "TRACE ",
    "DEBUG ",
    "INFO ",
    "WARN ",
    "ERROR ",
    "FATAL ",
};

void defualtOutput(const char* msg, int len) {
    size_t n = fwrite(msg, 1, len, stdout);
    (void)n;
}

void defualtFlush() {
    fflush(stdout);
}

Logger::OutputFunc g_output_func = defualtOutput;
Logger::FlushFunc g_flush_func = defualtFlush;

Logger::Logger(const char* file, int line) : _impl(Logger::INFO, file, line){}
Logger::Logger(const char* file, int line, LogLevel level) : _impl(level, file, line){}
Logger::Logger(const char* file, int line, LogLevel level, const char* func) 
        : _impl(level, file, line){
    _impl._stream << func << ' ';
}

Logger::~Logger() {
   _impl.finish(); 
   const LogStream::Buffer& buffer = stream().buffer();
   g_output_func(buffer.data(), buffer.length());
   if (_impl._level == Logger::FATAL) {
       g_flush_func();
       abort();
   }
}

Logger::LogLevel Logger::logLevel() {
    return g_loglevel;
}

void Logger::setLogLevel(LogLevel level) {
    g_loglevel = level;
}

void Logger::setOutput(OutputFunc out) {
    g_output_func = out;
}

void Logger::setFlush(FlushFunc flush) {
    g_flush_func = flush;
}

void Logger::Impl::formatTime() {
    int64_t microSecondsSinceEpoch = _timestamp.microSecondsSinceEpoch();
    time_t seconds = static_cast<time_t>(microSecondsSinceEpoch / 1000000);
    int microseconds = static_cast<int>(microSecondsSinceEpoch % 1000000);
    if (seconds != t_lastSecond) {
        t_lastSecond = seconds;
        struct tm tm_time;
        ::gmtime_r(&seconds, &tm_time);
        int len = snprintf(t_time, sizeof(t_time), "%4d%02d%02d %02d:%02d:%02d", 
                tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
        (void)len;
    }
    Fmt us(".%06dZ ", microseconds);
    _stream << t_time << us.data();
}

Logger::Impl::Impl(LogLevel level, const char* file, int line) 
        :_stream(), 
        _timestamp(Timestamp::now()), 
        _level(level),
        _line(line),
        _fullname(file), 
        _basename(NULL) {
    const char* path_sep_pos = strrchr(_fullname, '/');
    _basename = path_sep_pos == NULL ? _fullname : path_sep_pos + 1;
    formatTime();
    Fmt tid("%5d ", CurrentThread::tid());
    _stream << tid.data();
    _stream << LogLevelName[level];
}

void Logger::Impl::finish() {
    _stream << " - " << _basename << ':' << _line << '\n';
}

} // namespace rpc
} // namespace checkking
