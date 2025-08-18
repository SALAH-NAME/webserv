#include "SimpleLogger.hpp"

LogLevel SimpleLogger::_minLevel = LEVEL_INFO;

std::string SimpleLogger::getCurrentTime()
{
    std::time_t now = std::time(0);
    std::tm *timeinfo = std::localtime(&now);

    char buffer[20];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
    return std::string(buffer);
}

const char *SimpleLogger::getLevelColor(LogLevel level)
{
    switch (level)
    {
    case LEVEL_DEBUG:
        return COLOR_CYAN;
    case LEVEL_INFO:
        return COLOR_GREEN;
    case LEVEL_WARN:
        return COLOR_YELLOW;
    case LEVEL_ERROR:
        return COLOR_RED;
    case LEVEL_TRACE:
        return COLOR_MAGENTA;
    default:
        return COLOR_WHITE;
    }
}

const char *SimpleLogger::getLevelName(LogLevel level)
{
    switch (level)
    {
    case LEVEL_DEBUG:
        return "DEBUG";
    case LEVEL_INFO:
        return "INFO_";
    case LEVEL_WARN:
        return "WARNN";
    case LEVEL_ERROR:
        return "ERROR";
    case LEVEL_TRACE:
        return "TRACE";
    default:
        return "UNKN_";
    }
}

std::ostream &SimpleLogger::getOutput(LogLevel level)
{
    return (level >= LEVEL_ERROR) ? std::cerr : std::cout;
}

size_t SimpleLogger::countPlaceholders(const std::string &format)
{
    size_t count = 0;
    size_t pos = 0;

    while ((pos = format.find("{}", pos)) != std::string::npos)
    {
        count++;
        pos += 2;
    }

    return count;
}

std::string SimpleLogger::formatMessage(const std::string &format)
{
    return format;
}

void SimpleLogger::setLevel(LogLevel level)
{
    _minLevel = level;
}

void SimpleLogger::log(LogLevel level, const std::string &message)
{
    if (level < _minLevel)
        return;

    std::ostream &out = getOutput(level);
    out << getLevelColor(level) << "[" << getLevelName(level) << "] "
        << getCurrentTime() << " : " << message << COLOR_RESET << std::endl;
    out.flush();
}

void SimpleLogger::log(LogLevel level, const std::string &message, const std::string &clientIP)
{
    if (level < _minLevel)
        return;

    std::ostream &out = getOutput(level);
    out << getLevelColor(level) << "[" << getLevelName(level) << "] "
        << getCurrentTime() << " : CLIENT:" << clientIP << " " << message << COLOR_RESET << std::endl;
    out.flush();
}

void SimpleLogger::log(LogLevel level, const std::string &message, const std::string &clientIP, int port)
{
    if (level < _minLevel)
        return;

    std::ostream &out = getOutput(level);
    out << getLevelColor(level) << "[" << getLevelName(level) << "] "
        << getCurrentTime() << " : CLIENT:" << clientIP << ":" << toString(port) << " " << message << COLOR_RESET << std::endl;
    out.flush();
}

void SimpleLogger::logFormat(LogLevel level, const std::string &format)
{
    if (level < _minLevel)
        return;
    log(level, format);
}

void SimpleLogger::logRequest(const std::string &clientIP, int port, const std::string &method, const std::string &uri, const std::string &version, const std::string &extra)
{
    if (LEVEL_TRACE < _minLevel)
        return;

    std::ostream &out = getOutput(LEVEL_TRACE);
    out << COLOR_MAGENTA << "[TRACE] " << getCurrentTime() << " : CLIENT:" << clientIP << ":" << toString(port)
        << " " << method << " " << uri << " " << version << " REQUEST";
    if (!extra.empty())
        out << " " << extra;
    out << COLOR_RESET << std::endl;
    out.flush();
}

void SimpleLogger::logResponse(const std::string &clientIP, int port, const std::string &method, const std::string &uri, const std::string &version, int statusCode, const std::string &extra)
{
    if (LEVEL_TRACE < _minLevel)
        return;

    std::ostream &out = getOutput(LEVEL_TRACE);
    out << COLOR_MAGENTA << "[TRACE] " << getCurrentTime() << " : CLIENT:" << clientIP << ":" << toString(port)
        << " " << method << " " << uri << " " << version << " " << toString(statusCode) << " RESPONSE";
    if (!extra.empty())
        out << " " << extra;
    out << COLOR_RESET << std::endl;
    out.flush();
}

void SimpleLogger::logResponse(const std::string &clientIP, int port, const std::string &method, const std::string &uri, const std::string &version, int statusCode, size_t responseSize, const std::string &extra)
{
    if (LEVEL_TRACE < _minLevel)
        return;

    std::ostream &out = getOutput(LEVEL_TRACE);
    out << COLOR_MAGENTA << "[TRACE] " << getCurrentTime() << " : CLIENT:" << clientIP << ":" << toString(port)
        << " " << method << " " << uri << " " << version << " " << toString(statusCode)
        << " (" << toString(responseSize) << " bytes) RESPONSE";
    if (!extra.empty())
        out << " " << extra;
    out << COLOR_RESET << std::endl;
    out.flush();
}

void SimpleLogger::logError(const std::string &clientIP, int port, const std::string &method, const std::string &uri, const std::string &version, int statusCode, const std::string &error)
{
    if (LEVEL_ERROR < _minLevel)
        return;

    std::ostream &out = getOutput(LEVEL_ERROR);
    out << COLOR_RED << "[ERROR] " << getCurrentTime() << " : CLIENT:" << clientIP << ":" << toString(port)
        << " " << method << " " << uri << " " << version << " " << toString(statusCode) << " " << error << COLOR_RESET << std::endl;
    out.flush();
}

void SimpleLogger::logConfig(const std::string &message)
{
    log(LEVEL_INFO, message);
}

void SimpleLogger::logCgi(const std::string &message, const std::string &clientIP, int port)
{
    if (LEVEL_DEBUG < _minLevel)
        return;

    std::ostream &out = getOutput(LEVEL_DEBUG);
    out << COLOR_CYAN << "[DEBUG] " << getCurrentTime() << " : ";
    if (!clientIP.empty())
    {
        out << "CLIENT:" << clientIP;
        if (port > 0)
            out << ":" << toString(port);
        out << " ";
    }
    out << "CGI: " << message << COLOR_RESET << std::endl;
    out.flush();
}
