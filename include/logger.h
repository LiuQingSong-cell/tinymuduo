#ifndef LOGGER__H__
#define LOGGER__H__

#include "noncopyable.h"

#include <string>
#include <mutex>
#include <atomic>
#include <iostream>
#include <cstdio>

enum LogLevel
{
    INFO,
    ERROR,
    FATAL,
    DEBUG
};

#define LOG_INFO(logFormat, ...) \
    do \
    { \
        Logger* logger = Logger::getInstance(); \
        logger->setLogLevel(INFO); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024, logFormat, ##__VA_ARGS__); \
        logger->log(buf); \
    } while(0);

#define LOG_ERROR(logFormat, ...) \
    do \
    { \
        Logger* logger = Logger::getInstance(); \
        logger->setLogLevel(ERROR); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024, logFormat, ##__VA_ARGS__); \
        logger->log(buf); \
    } while(0);

#define LOG_FATAL(logFormat, ...) \
    do \
    { \
        Logger* logger = Logger::getInstance(); \
        logger->setLogLevel(FATAL); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024, logFormat, ##__VA_ARGS__); \
        logger->log(buf); \
        exit(-1);\
    } while(0);

#ifdef MUDEBUG
#define LOG_DEBUG(logFormat, ...) \
    do \
    { \
        Logger* logger = Logger::getInstance(); \
        logger->setLogLevel(DEBUG); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024, logFormat, ##__VA_ARGS__); \
        logger->log(buf); \
    } while(0);
#else
    #define LOG_DEBUG(logFormat, ...)
#endif


// thread safe sigleton
class Logger: noncopyable
{
    public:
        static Logger* getInstance();
        static std::atomic<Logger*> instance_;
        static std::mutex mutex_;
        ~Logger();

    public:
        void setLogLevel(LogLevel level);
        void log(std::string msg);

    private:
        LogLevel level_;    
};


#endif