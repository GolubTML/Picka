#pragma once

#include <android/log.h>
#include <string>
#include <fstream>
#include <mutex>

enum class Level 
{
    Debug,
    Info,
    Warning,
    Error
};

class Logger
{
public:
    Logger() { };
    ~Logger() = default;

    bool init(const std::string& logPath);

    void write(Level level, const char* tag, const char* format, ...);

private:
    std::ofstream logFile;
    std::mutex logMutex;
    std::string path;
};

extern Logger internalLogger;

#define INTERNAL_LOGGER "Picka"
#define LOGD(...) internalLogger.write(Level::Debug, INTERNAL_LOGGER, __VA_ARGS__)
#define LOGI(...) internalLogger.write(Level::Info, INTERNAL_LOGGER, __VA_ARGS__)
#define LOGW(...) internalLogger.write(Level::Warning, INTERNAL_LOGGER, __VA_ARGS__)
#define LOGE(...) internalLogger.write(Level::Error, INTERNAL_LOGGER, __VA_ARGS__)

extern Logger modLogger;

#define MOD_TAG "Mod"
#define M_LOGD(...) modLogger.write(Level::Debug, MOD_TAG, __VA_ARGS__)
#define M_LOGI(...) modLogger.write(Level::Info, MOD_TAG, __VA_ARGS__)
#define M_LOGW(...) modLogger.write(Level::Warning, MOD_TAG, __VA_ARGS__)
#define M_LOGE(...) modLogger.write(Level::Error, MOD_TAG, __VA_ARGS__)