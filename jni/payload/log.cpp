#include "log.h"

#include <cstdarg>
#include <cstdio>
#include <fstream>
#include <mutex>
#include <chrono>
#include <iomanip>

static const char* getLevelName(Level level)
{
    switch (level)
    {
    case Level::Debug:
        return "Debug";
        break;
    
    case Level::Info:
        return "Info";
        break;

    case Level::Warning:
        return "Warning";
        break;

    case Level::Error:
        return "Error";
        break;
    
    default:
        return "UKNOWN";
        break;
    }
}

static android_LogPriority getLogPriority(Level level)
{
    switch (level)
    {
    case Level::Debug:
        return ANDROID_LOG_DEBUG;
        break;
    
    case Level::Info:
        return ANDROID_LOG_INFO;
        break;

    case Level::Warning:
        return ANDROID_LOG_WARN;
        break;

    case Level::Error:
        return ANDROID_LOG_ERROR;
        break;
    
    default:
        return ANDROID_LOG_INFO;
        break;
    }
}

bool Logger::init(const std::string& logPath)
{
    std::lock_guard<std::mutex> lock(logMutex);
    
    logFile.open(logPath, std::ios::out | std::ios::app);

    if (!logFile.is_open())
    {
        __android_log_print(
            ANDROID_LOG_ERROR,
            "Picka",
            "Failed to open log file: %s",
            logPath.c_str()
        );

        return false;
    }
    else
    {
        __android_log_print(
            ANDROID_LOG_INFO,
            "Picka",
            "Log file opened: %s",
            logPath.c_str()
        );

        return true;
    }

    return false;
}

void Logger::write(Level level, const char* tag, const char* format, ...)
{   
    char message[4096];

    va_list args;
    va_start(args, format);

    vsnprintf(message, sizeof(message), format, args);
    
    va_end(args);

    __android_log_print(getLogPriority(level), tag, "%s", message);

    std::lock_guard<std::mutex> lock(logMutex);

    if (!logFile.is_open())
        return;

    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);

    std::tm tm{};

    localtime_r(&time, &tm);

    logFile 
        << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") 
        << " ["
        << getLevelName(level)
        << "] ["
        << tag
        << "] "
        << message
        << '\n';

    logFile.flush();

    if (!logFile.good())
    {
        __android_log_print(
            ANDROID_LOG_ERROR,
            "Picka",
            "Failed to write to log file!"
        );
    }
}