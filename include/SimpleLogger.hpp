#ifndef SIMPLE_LOGGER_HPP
#define SIMPLE_LOGGER_HPP

#include <iostream>
#include <string>
#include <ctime>
#include <sstream>
#include <vector>

#define COLOR_RESET "\033[0m"
#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_BLUE "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN "\033[36m"
#define COLOR_WHITE "\033[37m"
#define COLOR_BOLD "\033[1m"

enum LogLevel
{
    LEVEL_DEBUG = 0,
    LEVEL_INFO = 1,
    LEVEL_WARN = 2,
    LEVEL_ERROR = 3,
    LEVEL_TRACE = 4
};

class SimpleLogger
{
private:
    static LogLevel _minLevel;

    static std::string getCurrentTime();
    static const char *getLevelColor(LogLevel level);
    static const char *getLevelName(LogLevel level);
    static std::ostream &getOutput(LogLevel level);

    template <typename T>
    static std::string toString(const T &value);

    static size_t countPlaceholders(const std::string &format);

    static std::string formatMessage(const std::string &format);

    template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
    static std::string formatMessage(const std::string &format,
                                     const T1 &arg1, const T2 &arg2, const T3 &arg3, const T4 &arg4,
                                     const T5 &arg5, const T6 &arg6, const T7 &arg7, const T8 &arg8);

public:
    static void setLevel(LogLevel level);
    static void log(LogLevel level, const std::string &message);
    static void log(LogLevel level, const std::string &message, const std::string &clientIP);
    static void log(LogLevel level, const std::string &message, const std::string &clientIP, int port);

    static void logFormat(LogLevel level, const std::string &format);

    template <typename T1>
    static void logFormat(LogLevel level, const std::string &format, const T1 &arg1);

    template <typename T1, typename T2>
    static void logFormat(LogLevel level, const std::string &format, const T1 &arg1, const T2 &arg2);

    template <typename T1, typename T2, typename T3>
    static void logFormat(LogLevel level, const std::string &format, const T1 &arg1, const T2 &arg2, const T3 &arg3);

    template <typename T1, typename T2, typename T3, typename T4>
    static void logFormat(LogLevel level, const std::string &format, const T1 &arg1, const T2 &arg2, const T3 &arg3, const T4 &arg4);

    template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
    static void logFormat(LogLevel level, const std::string &format,
                          const T1 &arg1, const T2 &arg2, const T3 &arg3, const T4 &arg4,
                          const T5 &arg5, const T6 &arg6, const T7 &arg7, const T8 &arg8);

    static void logRequest(const std::string &clientIP, int port, const std::string &method, const std::string &uri, const std::string &version = "HTTP/1.1", const std::string &extra = "");
    static void logResponse(const std::string &clientIP, int port, const std::string &method, const std::string &uri, const std::string &version, int statusCode, const std::string &extra = "");
    static void logResponse(const std::string &clientIP, int port, const std::string &method, const std::string &uri, const std::string &version, int statusCode, size_t responseSize, const std::string &extra = "");
    static void logError(const std::string &clientIP, int port, const std::string &method, const std::string &uri, const std::string &version, int statusCode, const std::string &error);

    static void logConfig(const std::string &message);
    static void logCgi(const std::string &message, const std::string &clientIP = "", int port = 0);
};

template <typename T>
std::string SimpleLogger::toString(const T &value)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
std::string SimpleLogger::formatMessage(const std::string &format,
                                        const T1 &arg1, const T2 &arg2, const T3 &arg3, const T4 &arg4,
                                        const T5 &arg5, const T6 &arg6, const T7 &arg7, const T8 &arg8)
{
    std::vector<std::string> args;
    args.push_back(toString(arg1));
    args.push_back(toString(arg2));
    args.push_back(toString(arg3));
    args.push_back(toString(arg4));
    args.push_back(toString(arg5));
    args.push_back(toString(arg6));
    args.push_back(toString(arg7));
    args.push_back(toString(arg8));

    std::string result = format;
    size_t placeholderCount = std::min(countPlaceholders(format), 8UL);
    ;

    for (size_t i = 0; i < placeholderCount; ++i)
    {
        if (i < placeholderCount)
        {
            size_t pos = result.find("{}");
            if (pos != std::string::npos)
            {
                result.replace(pos, 2, args[i]);
            }
        }
    }

    return result;
}

template <typename T1>
void SimpleLogger::logFormat(LogLevel level, const std::string &format, const T1 &arg1)
{
    if (level < _minLevel)
        return;
    std::string message = formatMessage(format, arg1, std::string(), std::string(), std::string(), std::string(), std::string(), std::string(), std::string());
    log(level, message);
}

template <typename T1, typename T2>
void SimpleLogger::logFormat(LogLevel level, const std::string &format, const T1 &arg1, const T2 &arg2)
{
    if (level < _minLevel)
        return;
    std::string message = formatMessage(format, arg1, arg2, std::string(), std::string(), std::string(), std::string(), std::string(), std::string());
    log(level, message);
}

template <typename T1, typename T2, typename T3>
void SimpleLogger::logFormat(LogLevel level, const std::string &format, const T1 &arg1, const T2 &arg2, const T3 &arg3)
{
    if (level < _minLevel)
        return;
    std::string message = formatMessage(format, arg1, arg2, arg3, std::string(), std::string(), std::string(), std::string(), std::string());
    log(level, message);
}

template <typename T1, typename T2, typename T3, typename T4>
void SimpleLogger::logFormat(LogLevel level, const std::string &format, const T1 &arg1, const T2 &arg2, const T3 &arg3, const T4 &arg4)
{
    if (level < _minLevel)
        return;
    std::string message = formatMessage(format, arg1, arg2, arg3, arg4, std::string(), std::string(), std::string(), std::string());
    log(level, message);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
void SimpleLogger::logFormat(LogLevel level, const std::string &format,
                             const T1 &arg1, const T2 &arg2, const T3 &arg3, const T4 &arg4,
                             const T5 &arg5, const T6 &arg6, const T7 &arg7, const T8 &arg8)
{
    if (level < _minLevel)
        return;
    std::string message = formatMessage(format, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
    log(level, message);
}

#define LOG_INFO(msg) SimpleLogger::log(LEVEL_INFO, msg)
#define LOG_ERROR(msg) SimpleLogger::log(LEVEL_ERROR, msg)
#define LOG_WARN(msg) SimpleLogger::log(LEVEL_WARN, msg)
#define LOG_DEBUG(msg) SimpleLogger::log(LEVEL_DEBUG, msg)
#define LOG_TRACE(msg) SimpleLogger::log(LEVEL_TRACE, msg)

#define LOG_INFO_IP(msg, ip) SimpleLogger::log(LEVEL_INFO, msg, ip)
#define LOG_ERROR_IP(msg, ip) SimpleLogger::log(LEVEL_ERROR, msg, ip)
#define LOG_WARN_IP(msg, ip) SimpleLogger::log(LEVEL_WARN, msg, ip)
#define LOG_DEBUG_IP(msg, ip) SimpleLogger::log(LEVEL_DEBUG, msg, ip)

#define LOG_INFO_CLIENT(msg, ip, port) SimpleLogger::log(LEVEL_INFO, msg, ip, port)
#define LOG_ERROR_CLIENT(msg, ip, port) SimpleLogger::log(LEVEL_ERROR, msg, ip, port)
#define LOG_WARN_CLIENT(msg, ip, port) SimpleLogger::log(LEVEL_WARN, msg, ip, port)
#define LOG_DEBUG_CLIENT(msg, ip, port) SimpleLogger::log(LEVEL_DEBUG, msg, ip, port)

#define LOG_REQUEST(ip, port, method, uri) SimpleLogger::logRequest(ip, port, method, uri)
#define LOG_REQUEST_V(ip, port, method, uri, version) SimpleLogger::logRequest(ip, port, method, uri, version)
#define LOG_REQUEST_EXTRA(ip, port, method, uri, version, extra) SimpleLogger::logRequest(ip, port, method, uri, version, extra)

#define LOG_RESPONSE(ip, port, method, uri, version, status) SimpleLogger::logResponse(ip, port, method, uri, version, status)
#define LOG_RESPONSE_EXTRA(ip, port, method, uri, version, status, extra) SimpleLogger::logResponse(ip, port, method, uri, version, status, extra)
#define LOG_RESPONSE_SIZE(ip, port, method, uri, version, status, size) SimpleLogger::logResponse(ip, port, method, uri, version, status, size)
#define LOG_RESPONSE_SIZE_EXTRA(ip, port, method, uri, version, status, size, extra) SimpleLogger::logResponse(ip, port, method, uri, version, status, size, extra)

#define LOG_HTTP_ERROR(ip, port, method, uri, version, status, error) SimpleLogger::logError(ip, port, method, uri, version, status, error)

#define LOG_CONFIG(msg) SimpleLogger::logConfig(msg)
#define LOG_CGI(msg) SimpleLogger::logCgi(msg)
#define LOG_CGI_CLIENT(msg, ip, port) SimpleLogger::logCgi(msg, ip, port)

#define LOG_INFO_F(format, arg1) SimpleLogger::logFormat(LEVEL_INFO, format, arg1)
#define LOG_INFO_F2(format, arg1, arg2) SimpleLogger::logFormat(LEVEL_INFO, format, arg1, arg2)
#define LOG_INFO_F3(format, arg1, arg2, arg3) SimpleLogger::logFormat(LEVEL_INFO, format, arg1, arg2, arg3)
#define LOG_INFO_F4(format, arg1, arg2, arg3, arg4) SimpleLogger::logFormat(LEVEL_INFO, format, arg1, arg2, arg3, arg4)
#define LOG_INFO_F5(format, arg1, arg2, arg3, arg4, arg5) SimpleLogger::logFormat(LEVEL_INFO, format, arg1, arg2, arg3, arg4, arg5, std::string(), std::string(), std::string())
#define LOG_INFO_F6(format, arg1, arg2, arg3, arg4, arg5, arg6) SimpleLogger::logFormat(LEVEL_INFO, format, arg1, arg2, arg3, arg4, arg5, arg6, std::string(), std::string())
#define LOG_INFO_F7(format, arg1, arg2, arg3, arg4, arg5, arg6, arg7) SimpleLogger::logFormat(LEVEL_INFO, format, arg1, arg2, arg3, arg4, arg5, arg6, arg7, std::string())
#define LOG_INFO_F8(format, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) SimpleLogger::logFormat(LEVEL_INFO, format, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)

#define LOG_ERROR_F(format, arg1) SimpleLogger::logFormat(LEVEL_ERROR, format, arg1)
#define LOG_ERROR_F2(format, arg1, arg2) SimpleLogger::logFormat(LEVEL_ERROR, format, arg1, arg2)
#define LOG_ERROR_F3(format, arg1, arg2, arg3) SimpleLogger::logFormat(LEVEL_ERROR, format, arg1, arg2, arg3)
#define LOG_ERROR_F4(format, arg1, arg2, arg3, arg4) SimpleLogger::logFormat(LEVEL_ERROR, format, arg1, arg2, arg3, arg4)
#define LOG_ERROR_F5(format, arg1, arg2, arg3, arg4, arg5) SimpleLogger::logFormat(LEVEL_ERROR, format, arg1, arg2, arg3, arg4, arg5, std::string(), std::string(), std::string())
#define LOG_ERROR_F6(format, arg1, arg2, arg3, arg4, arg5, arg6) SimpleLogger::logFormat(LEVEL_ERROR, format, arg1, arg2, arg3, arg4, arg5, arg6, std::string(), std::string())
#define LOG_ERROR_F7(format, arg1, arg2, arg3, arg4, arg5, arg6, arg7) SimpleLogger::logFormat(LEVEL_ERROR, format, arg1, arg2, arg3, arg4, arg5, arg6, arg7, std::string())
#define LOG_ERROR_F8(format, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) SimpleLogger::logFormat(LEVEL_ERROR, format, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)

#define LOG_WARN_F(format, arg1) SimpleLogger::logFormat(LEVEL_WARN, format, arg1)
#define LOG_WARN_F2(format, arg1, arg2) SimpleLogger::logFormat(LEVEL_WARN, format, arg1, arg2)
#define LOG_WARN_F3(format, arg1, arg2, arg3) SimpleLogger::logFormat(LEVEL_WARN, format, arg1, arg2, arg3)
#define LOG_WARN_F4(format, arg1, arg2, arg3, arg4) SimpleLogger::logFormat(LEVEL_WARN, format, arg1, arg2, arg3, arg4)
#define LOG_WARN_F5(format, arg1, arg2, arg3, arg4, arg5) SimpleLogger::logFormat(LEVEL_WARN, format, arg1, arg2, arg3, arg4, arg5, std::string(), std::string(), std::string())
#define LOG_WARN_F6(format, arg1, arg2, arg3, arg4, arg5, arg6) SimpleLogger::logFormat(LEVEL_WARN, format, arg1, arg2, arg3, arg4, arg5, arg6, std::string(), std::string())
#define LOG_WARN_F7(format, arg1, arg2, arg3, arg4, arg5, arg6, arg7) SimpleLogger::logFormat(LEVEL_WARN, format, arg1, arg2, arg3, arg4, arg5, arg6, arg7, std::string())
#define LOG_WARN_F8(format, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) SimpleLogger::logFormat(LEVEL_WARN, format, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)

#define LOG_DEBUG_F(format, arg1) SimpleLogger::logFormat(LEVEL_DEBUG, format, arg1)
#define LOG_DEBUG_F2(format, arg1, arg2) SimpleLogger::logFormat(LEVEL_DEBUG, format, arg1, arg2)
#define LOG_DEBUG_F3(format, arg1, arg2, arg3) SimpleLogger::logFormat(LEVEL_DEBUG, format, arg1, arg2, arg3)
#define LOG_DEBUG_F4(format, arg1, arg2, arg3, arg4) SimpleLogger::logFormat(LEVEL_DEBUG, format, arg1, arg2, arg3, arg4)
#define LOG_DEBUG_F5(format, arg1, arg2, arg3, arg4, arg5) SimpleLogger::logFormat(LEVEL_DEBUG, format, arg1, arg2, arg3, arg4, arg5, std::string(), std::string(), std::string())
#define LOG_DEBUG_F6(format, arg1, arg2, arg3, arg4, arg5, arg6) SimpleLogger::logFormat(LEVEL_DEBUG, format, arg1, arg2, arg3, arg4, arg5, arg6, std::string(), std::string())
#define LOG_DEBUG_F7(format, arg1, arg2, arg3, arg4, arg5, arg6, arg7) SimpleLogger::logFormat(LEVEL_DEBUG, format, arg1, arg2, arg3, arg4, arg5, arg6, arg7, std::string())
#define LOG_DEBUG_F8(format, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) SimpleLogger::logFormat(LEVEL_DEBUG, format, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)

#define LOG_TRACE_F(format, arg1) SimpleLogger::logFormat(LEVEL_TRACE, format, arg1)
#define LOG_TRACE_F2(format, arg1, arg2) SimpleLogger::logFormat(LEVEL_TRACE, format, arg1, arg2)
#define LOG_TRACE_F3(format, arg1, arg2, arg3) SimpleLogger::logFormat(LEVEL_TRACE, format, arg1, arg2, arg3)
#define LOG_TRACE_F4(format, arg1, arg2, arg3, arg4) SimpleLogger::logFormat(LEVEL_TRACE, format, arg1, arg2, arg3, arg4)
#define LOG_TRACE_F5(format, arg1, arg2, arg3, arg4, arg5) SimpleLogger::logFormat(LEVEL_TRACE, format, arg1, arg2, arg3, arg4, arg5, std::string(), std::string(), std::string())
#define LOG_TRACE_F6(format, arg1, arg2, arg3, arg4, arg5, arg6) SimpleLogger::logFormat(LEVEL_TRACE, format, arg1, arg2, arg3, arg4, arg5, arg6, std::string(), std::string())
#define LOG_TRACE_F7(format, arg1, arg2, arg3, arg4, arg5, arg6, arg7) SimpleLogger::logFormat(LEVEL_TRACE, format, arg1, arg2, arg3, arg4, arg5, arg6, arg7, std::string())
#define LOG_TRACE_F8(format, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) SimpleLogger::logFormat(LEVEL_TRACE, format, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)

#define SET_LOG_LEVEL(level) SimpleLogger::setLevel(level)

#define LOGGER_F1(level, format, arg1) SimpleLogger::logFormat(level, format, arg1)
#define LOGGER_F2(level, format, arg1, arg2) SimpleLogger::logFormat(level, format, arg1, arg2)
#define LOGGER_F3(level, format, arg1, arg2, arg3) SimpleLogger::logFormat(level, format, arg1, arg2, arg3)
#define LOGGER_F4(level, format, arg1, arg2, arg3, arg4) SimpleLogger::logFormat(level, format, arg1, arg2, arg3, arg4)
#define LOGGER_F5(level, format, arg1, arg2, arg3, arg4, arg5) SimpleLogger::logFormat(level, format, arg1, arg2, arg3, arg4, arg5, std::string(), std::string(), std::string())
#define LOGGER_F6(level, format, arg1, arg2, arg3, arg4, arg5, arg6) SimpleLogger::logFormat(level, format, arg1, arg2, arg3, arg4, arg5, arg6, std::string(), std::string())
#define LOGGER_F7(level, format, arg1, arg2, arg3, arg4, arg5, arg6, arg7) SimpleLogger::logFormat(level, format, arg1, arg2, arg3, arg4, arg5, arg6, arg7, std::string())
#define LOGGER_F8(level, format, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) SimpleLogger::logFormat(level, format, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)

inline void logInfo(const std::string &msg) { LOG_INFO(msg); }
inline void logError(const std::string &msg) { LOG_ERROR(msg); }
inline void logWarn(const std::string &msg) { LOG_WARN(msg); }
inline void logDebug(const std::string &msg) { LOG_DEBUG(msg); }
inline void logConfig(const std::string &msg) { LOG_CONFIG(msg); }

inline void logRequest(const std::string &ip, int port, const std::string &method, const std::string &uri)
{
    LOG_REQUEST(ip, port, method, uri);
}

inline void logResponse(const std::string &ip, int port, const std::string &method, const std::string &uri, const std::string &version, int status)
{
    LOG_RESPONSE(ip, port, method, uri, version, status);
}

inline void logHttpError(const std::string &ip, int port, const std::string &method, const std::string &uri, const std::string &version, int status, const std::string &error)
{
    LOG_HTTP_ERROR(ip, port, method, uri, version, status, error);
}

#endif
