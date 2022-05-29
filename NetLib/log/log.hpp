//
// Created by Cmf on 2022/5/23.
//

#ifndef CMFNETLIB_LOG_HPP
#define CMFNETLIB_LOG_HPP

#include "NetLib/base/noncopyable.h"
#include <memory>
#include <chrono>
#include <thread>
#include <sstream>

/*
 * 日志级别
 */
enum class LogLevel : uint8_t {
    NONE = 0,
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4,
    FATAL = 5
};

class Logger;

/*
 * 日志事件
 */
class LogEvent {
public:
    using ptr = std::shared_ptr<LogEvent>;

    LogEvent(const std::string &level, const std::string &fileName, const uint32_t line, const std::string &time,
             const std::string &msg,
             const std::string &threadName = "")
            : _level(FromString(level)), _fileName(fileName), _line(line), _time(time),
              _msg(msg),
              _threadName(threadName),
              _threadId(std::this_thread::get_id()) {
    }

    static const char *ToString(LogLevel level) noexcept {
        switch (level) {
            case LogLevel::DEBUG:
                return "DEBUG";
                break;
            case LogLevel::INFO:
                return "INFO";
                break;
            case LogLevel::WARN:
                return "WARN";
                break;
            case LogLevel::ERROR:
                return "ERROR";
                break;
            case LogLevel::FATAL:
                return "FATAL";
                break;
            default:
                return "NONE";
        }
    }

    static LogLevel FromString(const std::string &str) noexcept {
        if (str == "DEBUG") {
            return LogLevel::DEBUG;
        } else if (str == "INFO") {
            return LogLevel::INFO;
        } else if (str == "WARN") {
            return LogLevel::WARN;
        } else if (str == "ERROR") {
            return LogLevel::ERROR;
        } else if (str == "FATAL") {
            return LogLevel::FATAL;
        } else {
            return LogLevel::NONE;
        }
    }

    LogLevel GetLevel() const {
        return _level;
    }

    std::thread::id GetThreadId() const {
        return _threadId;
    }

    std::string GetFileName() const {
        return _fileName;
    }

    uint32_t GetLine() const {
        return _line;
    }

    std::string GetMsg() const {
        return _msg;
    }

    std::string GetTime() const {
        return _time;
    }

private:
    uint32_t _line = 0;
    std::thread::id _threadId;
    std::string _threadName;
    std::string _fileName;
    std::string _time;
    std::string _msg;
    LogLevel _level;
    std::stringstream _ss;
    std::shared_ptr<Logger> _logger;

};

/*
 * 日志器
 */
class Logger : private noncopyable {
public:
    using ptr = std::shared_ptr<Logger>;

    static Logger &GetInstance() {
        static Logger logger;
        return logger;
    }

    const std::string GetCurrentSystemTime() {
        auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        struct tm *ptm = localtime(&t);
        char date[60] = {0};
        sprintf(date, "%4d-%02d-%02d %02d:%02d:%02d",
                static_cast<int>(ptm->tm_year + 1900),
                static_cast<int>(ptm->tm_mon + 1),
                static_cast<int>(ptm->tm_mday),
                static_cast<int>(ptm->tm_hour),
                static_cast<int>(ptm->tm_min),
                static_cast<int>(ptm->tm_sec));
        return std::move(std::string(date));
    }

    void Log(const LogEvent::ptr event) {
        std::cout << event->GetTime() << " "
                  << event->ToString(event->GetLevel()) << " "
                  << event->GetThreadId() << " "
                  << event->GetFileName() << " "
                  << event->GetLine() << " "
                  << event->GetMsg() << " " << std::endl;
    }

private:
    std::shared_ptr<LogEvent::ptr> _logEvent;
};

#endif //CMFNETLIB_LOG_HPP
