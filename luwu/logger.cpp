//
// Created by liucxi on 2022/10/31.
//

#include "logger.h"
#include <cstdarg>          // for va_list
#include <cstdio>           // for va_list
#include <utility>
#include <functional>
#include <iostream>

namespace luwu {
    std::string LogLevel::ToString(LogLevel::Level level) {
#define XX(name, v) if (level == v) { return #name; }
        XX(DEBUG, DEBUG);
        XX(INFO, INFO);
        XX(ERROR, ERROR);
        XX(FATAL, FATAL);
#undef XX
        return "UNKNOWN";
    }

    LogLevel::Level LogLevel::FromString(const std::string &level) {
#define XX(name, v) if (level == #v) { return LogLevel::name; }
        XX(DEBUG, DEBUG);
        XX(INFO, INFO);
        XX(ERROR, ERROR);
        XX(FATAL, FATAL);
#undef XX
        return LogLevel::UNKNOWN;
    }

    // region # LogEvent::LogEvent()
    LogEvent::LogEvent(LogLevel::Level level, std::string loggerName,
                       std::string file, std::string funcName, uint32_t line,
                       std::string threadName, uint32_t elapse,
                       uint64_t time, uint32_t tid, uint32_t fid)
            : level_(level), logger_name_(std::move(loggerName)),
              file_(std::move(file)), func_name_(std::move(funcName)),
              line_(line), thread_name_(std::move(threadName)),
              elapse_(elapse), time_(time), tid_(tid), fid_(fid) {
    }
    // endregion

    // 这四行代码为解决不定参数格式化输出的模板
    void LogEvent::Print(const char *fmt, ...) {
        va_list va;
        va_start(va, fmt);
        vprintf(fmt, va);
        va_end(va);
    }

    // region # LogFormatter 的各个继承类

    class MessageFormatItem : public LogFormatter::FormatItem {
    public:
        // 大部分子类的构造参数都没有作用，是为了与需要 string 成员的类保持一致而存在的，只有两个 DateFormatItem 和 StringFormatItem
        explicit MessageFormatItem(const std::string &str) {}

        void format(std::ostream &os, LogEvent::ptr event) override {
            os << event->getMessage();
        }
    };

    class LogLevelFormatItem : public LogFormatter::FormatItem {
    public:
        explicit LogLevelFormatItem(const std::string &str) {}

        void format(std::ostream &os, LogEvent::ptr event) override {
            os << LogLevel::ToString(event->getLevel());
        }
    };

    class LoggerNameFormatItem : public LogFormatter::FormatItem {
    public:
        explicit LoggerNameFormatItem(const std::string &str) {}

        void format(std::ostream &os, LogEvent::ptr event) override {
            os << event->getLoggerName();
        }
    };

    class DateFormatItem : public LogFormatter::FormatItem {
    public:
        explicit DateFormatItem(std::string str = "%Y-%m-%d %H:%M:%S")
                : pattern_(std::move(str)) {
            if (pattern_.empty()) {
                pattern_ = "%Y-%m-%d %H:%M:%S";
            }
        }

        void format(std::ostream &os, LogEvent::ptr event) override {
            auto time = static_cast<time_t>(event->getTime());
            struct tm tm{};
            localtime_r(&time, &tm);  // 将给定的时间戳(time)转换为本地时间(tm)
            char buf[64];
            strftime(buf, sizeof buf, pattern_.c_str(), &tm);  // 格式化时间表示
            os << buf;
        }

    private:
        std::string pattern_;
    };

    class ElapseFormatItem : public LogFormatter::FormatItem {
    public:
        explicit ElapseFormatItem(const std::string &str) {}

        void format(std::ostream &os, LogEvent::ptr event) override {
            os << event->getElapse();
        }
    };

    class FileFormatItem : public LogFormatter::FormatItem {
    public:
        explicit FileFormatItem(const std::string &str) {}

        void format(std::ostream &os, LogEvent::ptr event) override {
            os << event->getFile();
        }
    };

    class FuncNameFormatItem : public LogFormatter::FormatItem {
    public:
        explicit FuncNameFormatItem(const std::string &str) {}

        void format(std::ostream &os, LogEvent::ptr event) override {
            os << event->getFuncName();
        }
    };

    class LineFormatItem : public LogFormatter::FormatItem {
    public:
        explicit LineFormatItem(const std::string &str) {}

        void format(std::ostream &os, LogEvent::ptr event) override {
            os << event->getLine();
        }
    };

    class TidFormatItem : public LogFormatter::FormatItem {
    public:
        explicit TidFormatItem(const std::string &str) {}

        void format(std::ostream &os, LogEvent::ptr event) override {
            os << event->getTid();
        }
    };

    class FidFormatItem : public LogFormatter::FormatItem {
    public:
        explicit FidFormatItem(const std::string &str) {}

        void format(std::ostream &os, LogEvent::ptr event) override {
            os << event->getFid();
        }
    };

    class ThreadNameFormatItem : public LogFormatter::FormatItem {
    public:
        explicit ThreadNameFormatItem(const std::string &str) {}

        void format(std::ostream &os, LogEvent::ptr event) override {
            os << event->getThreadName();
        }
    };

    class TabFormatItem : public LogFormatter::FormatItem {
    public:
        explicit TabFormatItem(const std::string &str) {}

        void format(std::ostream &os, LogEvent::ptr event) override {
            os << "\t";
        }
    };

    class NewLineFormatItem : public LogFormatter::FormatItem {
    public:
        explicit NewLineFormatItem(const std::string &str) {}

        void format(std::ostream &os, LogEvent::ptr event) override {
            os << "\n";
        }
    };

    class StringFormatItem : public LogFormatter::FormatItem {
    public:
        explicit StringFormatItem(std::string str) : content_(std::move(str)) {}

        void format(std::ostream &os, LogEvent::ptr event) override {
            os << content_;
        }

    private:
        std::string content_;
    };

    // endregion

    // region # LogFormatter::LogFormatter()
    LogFormatter::LogFormatter(std::string pattern)
            : error_(false), pattern_(std::move(pattern)) {
        init();
    }
    // endregion

    // 状态机来实现字符串的解析
    void LogFormatter::init() {
        // 按顺序存储解析到的 pattern 项
        // 每个 pattern 包括一个整数类型和一个字符串，类型为 0 表示该 pattern 是常规字符串，为 1 表示该 pattern 需要转义
        // 日期格式单独用下面的 data_format 存储
        std::vector<std::pair<int, std::string>> patterns;
        // 临时存储常规字符串
        std::string tmp;
        // 日期格式字符串，默认把 %d 后面的内容全部当作格式字符串，不校验是否合法
        std::string data_format;
        // 是否解析出错
        bool error = false;
        // 正在解析常规字符串
        bool parsing_string = true;

        size_t i = 0;
        while (i < pattern_.size()) {
            std::string c = std::string(1, pattern_[i]);

            if (c == "%") {
                if (parsing_string) {
                    if (!tmp.empty()) {
                        patterns.emplace_back(0, tmp); // 在解析常规字符时遇到 %，表示开始解析模板字符
                    }
                    tmp.clear();
                    parsing_string = false;
                    ++i;
                    continue;
                }
                patterns.emplace_back(1, c); // 在解析模板字符时遇到 %，表示这里是一个转义字符
                parsing_string = true;
                ++i;
                continue;
            } else {
                if (parsing_string) {
                    tmp += c;
                    ++i;
                    continue;
                }
                patterns.emplace_back(1, c); // 模板字符直接添加，因为模板字符只有 1 个字母
                parsing_string = true;
                if (c != "d") {
                    ++i;
                    continue;
                }

                // 下面是对 %d 的特殊处理，直接取出 { } 内内容，不校验格式
                ++i;
                if (i < pattern_.size() && pattern_[i] != '{') {
                    continue; //不符合规范，不是 {
                }
                ++i;
                while (i < pattern_.size() && pattern_[i] != '}') {
                    data_format.push_back(pattern_[i]);
                    ++i;
                }
                if (pattern_[i] != '}') {
                    error = true;
                    break; //不符合规范，不是 }
                }
                ++i;
                continue;
            }
        } // end while
        if (error) {
            error_ = true;
            return;
        }
        // 模板解析最后的常规字符也要记得加进去
        if (!tmp.empty()) {
            patterns.emplace_back(0, tmp);
            tmp.clear();
        }

        // 构造一个 unordered_map，ump = {{"0", std::function}, {"1", std::function}}
        // std::function 接受一个字符串，返回一个同名的类对象
        // 例如：XX(m, MessageFormatItem)
        // {”m”, [](const std::string &fmt) { return FormatItem::ptr(new MessageFormatItem(fmt)); }}
        static std::unordered_map<std::string, std::function<FormatItem::ptr(const std::string)>> s_format_items = {
#define XX(str, C) { #str, [](const std::string &fmt) { return FormatItem::ptr(new C(fmt)); } },
                XX(m, MessageFormatItem)
                XX(p, LogLevelFormatItem)
                XX(c, LoggerNameFormatItem)
                XX(r, ElapseFormatItem)
                XX(f, FileFormatItem)
                XX(a, FuncNameFormatItem)
                XX(l, LineFormatItem)
                XX(t, TidFormatItem)
                XX(b, FidFormatItem)
                XX(n, ThreadNameFormatItem)
                XX(N, NewLineFormatItem)
                XX(T, TabFormatItem)
#undef XX
        };

        for (const auto &v: patterns) {
            if (v.first == 0) {             // 常规字符串，使用 StringFormatItem 对象
                items_.push_back(FormatItem::ptr(new StringFormatItem(v.second)));
            } else if (v.second == "d") {   // 时间，使用 DateFormatItem 对象
                items_.push_back(FormatItem::ptr(new DateFormatItem(data_format)));
            } else {                        // 其他格式，在 map 里面查找
                auto it = s_format_items.find(v.second);
                if (it == s_format_items.end()) {
                    error = true;
                    break;
                } else {
                    items_.push_back(it->second(v.second));
                }
            }
        }
        if (error) {
            error_ = true;
            return;
        }
    }

    std::string LogFormatter::format(LogEvent::ptr &event) {
        std::stringstream ss;
        for (const auto &item: items_) {
            item->format(ss, event);
        }
        return ss.str();
    }

    std::ostream &LogFormatter::format(std::ostream &os, LogEvent::ptr &event) {
        for (const auto &item: items_) {
            item->format(os, event);
        }
        return os;
    }

    void StdoutLogAppender::log(LogEvent::ptr event) {
        SpinLock::Lock lock(mutex_);
        formatter_->format(std::cout, event);
    }

    // region # FileLogAppender::FileLogAppender()
    FileLogAppender::FileLogAppender(std::string filename)
        : LogAppender(std::make_shared<LogFormatter>())
        , filename_(std::move(filename)), reopen_error_(false)
        , last_open_time_(0) {
        reopen();
    }
    // endregion

    bool FileLogAppender::reopen() {
        SpinLock::Lock lock(mutex_);
        if (filestream_) {
            filestream_.close();
        }
        filestream_.open(filename_);
        reopen_error_ = !filestream_;
        return !reopen_error_;
    }

    void FileLogAppender::log(LogEvent::ptr event) {
        // 如果一个文件打开超过 3 秒就重新打开一次，确保日志文件可以正常写入
        uint64_t now = event->getTime();
        if (now > last_open_time_ + 3) {
            reopen();
            last_open_time_ = now;
        }
        if (reopen_error_) {
            return;
        }
        SpinLock::Lock lock(mutex_);
        formatter_->format(filestream_, event);
    }

    // region # Logger::Logger()
    Logger::Logger(std::string logger_name, LogLevel::Level logger_level)
        : logger_name_(std::move(logger_name)), logger_level_(logger_level), create_time_(getElapseMs()) {
    }
    // endregion

    void Logger::addAppender(const LogAppender::ptr& appender) {
        SpinLock::Lock lock(mutex_);
        appenders_.push_back(appender);
    }

    void Logger::delAppender(const LogAppender::ptr& appender) {
        SpinLock::Lock lock(mutex_);
        appenders_.remove(appender);
    }

    void Logger::clearAppender() {
        SpinLock::Lock lock(mutex_);
        appenders_.clear();
    }

    void Logger::log(const LogEvent::ptr& event) {
        if (event->getLevel() >= logger_level_) {
            for (const auto &appender : appenders_) {
                appender->log(event);
            }
        }
    }

    LoggerManager::LoggerManager() {
        root_logger_.reset(new Logger("root", LogLevel::DEBUG));  // smart ptr 的 reset
        root_logger_->addAppender(LogAppender::ptr(new StdoutLogAppender()));  // 主日志器默认标准输出，默认格式
        loggers_["root"] = root_logger_;
    }

    Logger::ptr LoggerManager::getRootLogger() {
        return root_logger_;
    }

    Logger::ptr LoggerManager::getLogger(const std::string &name) {
        SpinLock::Lock lock(mutex_);
        auto it = loggers_.find(name);
        if (it == loggers_.end()) {
            loggers_[name] = std::make_shared<Logger>(name);
        }
        return loggers_[name];
    }
}