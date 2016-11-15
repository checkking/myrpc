#ifndef CHECKKING_RPC_BASE_LOGGING_H
#define CHECKKING_RPC_BASE_LOGGING_H

#include "LogStream.h"
#include "Timestamp.h"

namespace checkking {
namespace rpc {
class Logger {
public:
    enum LogLevel {
        TRACE,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
        NUM_LOG_LEVELS,
    };
    Logger(const char* file, int line);
    Logger(const char* file, int line, LogLevel level);
    Logger(const char* file, int line, LogLevel level, const char* func);
    ~Logger();
    LogStream& stream() {
        return _impl._stream;
    }

    static LogLevel logLevel();
    static void setLogLevel(LogLevel level);

    typedef void (*OutputFunc)(const char* msg, int len);
    typedef void (*FlushFunc)();
    static void setOutput(OutputFunc);
    static void setFlush(FlushFunc); 

private:
class Impl {
typedef Logger::LogLevel LogLevel;
public:
   Impl(LogLevel, const char* file, int line);
   void formatTime();
   void finish();

   LogStream _stream; 
   Timestamp _timestamp;
   LogLevel _level;
   int _line;
   const char* _fullname;
   const char* _basename;
}; // class Impl

Impl _impl;
}; // class Logger

#define LOG_TRACE if (Logger::logLevel() <= Logger::TRACE) \
        Logger(__FILE__, __LINE__, Logger::TRACE, __func__).stream()
#define LOG_DEBUG if (Logger::logLevel() <= Logger::DEBUG) \
        Logger(__FILE__, __LINE__, Logger::DEBUG, __func__).stream()
#define LOG_INFO if (Logger::logLevel() <= Logger::INFO) \
        Logger(__FILE__, __LINE__, Logger::INFO, __func__).stream()
#define LOG_WARN if (Logger::logLevel() <= Logger::WARN) \
        Logger(__FILE__, __LINE__, Logger::WARN, __func__).stream()
#define LOG_ERROR if (Logger::logLevel() <= Logger::ERROR) \
        Logger(__FILE__, __LINE__, Logger::ERROR, __func__).stream()
#define LOG_FATAL if (Logger::logLevel() <= Logger::FATAL) \
        Logger(__FILE__, __LINE__, Logger::FATAL, __func__).stream()

} // namespace rpc
} // namespace checkking
#endif // CHECKKING_RPC_BASE_LOGGING_H
