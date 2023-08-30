#include "logger.h"
#include "Timestamp.h"

std::atomic<Logger*> Logger::instance_;
std::mutex Logger::mutex_;


Logger* Logger::getInstance()
{
    Logger* tmp = instance_.load(std::memory_order_acquire);
    if (tmp == nullptr)
    {
        std::lock_guard<std::mutex> guard(mutex_);
        tmp = instance_.load(std::memory_order_acquire);
        if (tmp == nullptr)
        {
            tmp = new Logger;
            instance_.store(tmp, std::memory_order_release);
        }
    }
    return tmp;
}

void Logger::setLogLevel(LogLevel level)
{
    level_ = level;
}

void Logger::log(std::string msg)
{
    std::string prefix = "";

   switch (level_)
   {
    case INFO:
        prefix = "[INFO]";
        break;
    case ERROR:
        prefix = "[ERROR]";
        break;
    case FATAL:
        prefix = "[FATAL]";
        break;
    case DEBUG:
        prefix = "[DEBUG]";
        break;
   default:
    break;
   }

   std::cout << prefix + Timestamp::now().toString() + ": " + msg << std::endl;
}

Logger::~Logger()
{
    Logger* tmp = instance_.load();
    if (tmp != nullptr)
    {
        std::lock_guard<std::mutex> guard(mutex_);
        tmp = instance_.load();
        if (tmp != nullptr)
        {
            delete tmp;
            instance_ = nullptr;
        }
    }
}
