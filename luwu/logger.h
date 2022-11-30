//
// Created by liucxi on 2022/10/31.
//

#ifndef LUWU_LOGGER_H
#define LUWU_LOGGER_H

#include <memory>
#include <string>
#include <sstream>
#include <fstream>
#include <list>
#include <vector>
#include <unordered_map>
#include "utils/singleton.h"
#include "utils/mutex.h"
#include "utils/util.h"

// region # 宏定义获取日志器
#define LUWU_LOG_ROOT() LoggerMgr::GetInstance().getRootLogger()

#define LUWU_LOG_NAME(name) LoggerMgr::GetInstance().getLogger(name)
// endregion

// region # 宏定义流式输出
#define LUWU_LOG(logger, level)                                                                             \
        if ((level) >= (logger)->getLoggerLevel())                                                          \
            luwu::LogEventWrapper(                                                                          \
                logger,                                                                                     \
                luwu::LogEvent::ptr(                                                                        \
                             new luwu::LogEvent(level, (logger)->getLoggerName(),                           \
                                 __FILE__, __FUNCTION__, __LINE__,                                          \
                                 luwu::getThreadName(), luwu::getElapseMs() - (logger)->getCreateTime(),    \
                                 time(nullptr), luwu::getThreadId(), luwu::getFiberId()                     \
                             )                                                                              \
                                )).getLogEvent()->getMessageStream()                                        \

#define LUWU_LOG_DEBUG(logger) LUWU_LOG(logger, LogLevel::DEBUG)

#define LUWU_LOG_INFO(logger) LUWU_LOG(logger, LogLevel::INFO)

#define LUWU_LOG_ERROR(logger) LUWU_LOG(logger, LogLevel::ERROR)

#define LUWU_LOG_FATAL(logger) LUWU_LOG(logger, LogLevel::FATAL)
// endregion

// region # 宏定义 fmt 输出
#define LUWU_LOG_FMT(logger, level, fmt, ...)                                                       \
        if ((level) >= (logger)->getLoggerLevel())                                                  \
            LogEventWrapper(                                                                        \
                logger,                                                                             \
                LogEvent::ptr(                                                                      \
                    new LogEvent(level, (logger)->getLoggerName(),                                  \
                                 __FILE__, __FUNCTION__, __LINE__,                                  \
                                 getThreadName(), getElapseMs() - (logger)->getCreateTime(),        \
                                 time(nullptr), getThreadId(), getFiberId()                         \
                             )                                                                      \
                                )).getLogEvent()->Print(fmt, __VA_ARGS__);                          \

#define LUWU_LOG_FMT_DEBUG(logger, fmt, ...) LUWU_LOG_FMT(logger, LogLevel::DEBUG, fmt, __VA_ARGS__)

#define LUWU_LOG_FMT_INFO(logger, fmt, ...) LUWU_LOG_FMT(logger, LogLevel::INFO, fmt, __VA_ARGS__)

#define LUWU_LOG_FMT_ERROR(logger, fmt, ...) LUWU_LOG_FMT(logger, LogLevel::ERROR, fmt, __VA_ARGS__)

#define LUWU_LOG_FMT_FATAL(logger, fmt, ...) LUWU_LOG_FMT(logger, LogLevel::FATAL, fmt, __VA_ARGS__)
// endregion

namespace luwu {

    /**
     * @brief 日志等级
     */
    class LogLevel {
    public:
        /**
         * @brief 默认输出该级别及以上的日志，设置为 UNKNOWN 则不会输出任何日志
         */
        enum Level {
            DEBUG,
            INFO,
            ERROR,
            FATAL,
            UNKNOWN,
        };

        /**
         * @brief LogLevel::Level 转 std::string
         * @param level 日志等级
         * @return 转换后的字符串
         */
        static std::string ToString(LogLevel::Level level);

        /**
         * @brief std::string 转 LogLevel::Level
         * @param level 日志等级字符串
         * @return 转换后的 Level
         */
        static LogLevel::Level FromString(const std::string &level);
    };

    /**
     * @brief 日志现场，保存日志现场的所有信息
     */
    class LogEvent {
    public:
        using ptr = std::shared_ptr<LogEvent>;

        /**
         * @brief 构造函数
         * @param level 日志等级
         * @param loggerName 打印该日志现场的日志器名称
         * @param file 日志现场 -- 文件名
         * @param funcName 日志现场 -- 函数名
         * @param line 日志现场 -- 行号
         * @param threadName 线程名
         * @param elapse 系统启动到现在的毫秒数
         * @param time 时间戳
         * @param tid 线程 id
         * @param fid 协程 id
         */
        LogEvent(LogLevel::Level level, std::string loggerName,
                 std::string file, std::string funcName, uint32_t line,
                 std::string threadName,
                 uint32_t elapse, uint64_t time, uint32_t tid, uint32_t fid);

        /**
         * @brief 实现了不定参数的格式化输出
         * @param fmt 格式化字符串
         * @param ... 不定参数
         */
        static void Print(const char *fmt, ...);

        /**
         * @brief 获取 std::stringstream 字符串流
         * @details 用于宏定义形式的流式输出，该字符串流保存用户给定的日志信息
         * @note 不可返回 const 类型，因为在宏定义的最后会向字符串流输入
         * @return 字符串流
         */
        std::stringstream &getMessageStream() { return message_; }

        // region ## Getter

        LogLevel::Level getLevel() const { return level_; }

        const std::string &getLoggerName() const { return logger_name_; }

        std::string getMessage() const { return message_.str(); }

        const std::string &getFile() const { return file_; }

        const std::string &getFuncName() const { return func_name_; }

        uint32_t getLine() const { return line_; }

        const std::string &getThreadName() const { return thread_name_; }

        uint32_t getElapse() const { return elapse_; }

        uint64_t getTime() const { return time_; }

        uint32_t getTid() const { return tid_; }

        uint32_t getFid() const { return fid_; }

        // endregion

    private:
        /// 日志等级
        LogLevel::Level level_;
        /// 所属的日志器，以后可以分辨出是由哪一个日志器打印的
        std::string logger_name_;
        /// 日志现场信息，使用 ss，方便以后进行流式输出
        std::stringstream message_;
        /// 日志现场 --- 文件名
        std::string file_;
        /// 日志现场 --- 函数名
        std::string func_name_;
        /// 日志现场 --- 行号
        uint32_t line_;
        /// 日志现场 --- 线程名
        std::string thread_name_;
        /// 程序启动到现在的毫秒数
        uint32_t elapse_;
        /// 时间戳
        uint64_t time_;
        /// 线程 id
        uint32_t tid_;
        /// 协程 id
        uint32_t fid_;
    };

    /**
     * @brief 日志格式器，用来将一个日志现场转化成指定的格式
     */
    class LogFormatter {
    public:
        using prt = std::shared_ptr<LogFormatter>;

        /**
         * @brief 构造函数
         * @param pattern 日志的打印格式
         * @details 格式参数说明
         * - %m 消息
         * - %p 日志级别
         * - %c 日志器名称
         * - %d 日期时间，后面跟一对括号指定时间格式，如 %d{%Y-%m-%d %H:%M:%S}
         * - %r 程序运行到现在的毫秒数
         * - %f 文件名
         * - %a 函数名
         * - %l 行号
         * - %t 线程id
         * - %b 协程id
         * - %n 线程名称
         * - %T 制表符
         * - %N 换行
         * 默认格式：" [%p] [%c] %d{%Y-%m-%d %H:%M:%S} [%rms]%T [thread_id: %t] [thread_name: %n] [fiber_id: %b]%T%f:%a:%l%T%m%N"
         * 格式描述： [日志级别] [日志器名称] 年-月-日 时:分:秒 [累计运行毫秒数] 线程id 线程名称 协程id  文件名:函数名:行号 日志消息
         */
        explicit LogFormatter(std::string pattern =" [%p] [%c] %d{%Y-%m-%d %H:%M:%S} %f:%l%T%m%N");

        /**
         * @brief 解析 pattern 并构造 FormatItems
         */
        void init();

        /**
         * @brief 将日志现场格式化生成一个字符串
         * @param event 日志现场
         * @return 根据 pattern 生成的日志现场字符串
         */
        std::string format(LogEvent::ptr &event);

        /**
         * @brief 将日志现场格式化到输出流中
         * @param os 输出流，如 std::cout, std::stringstream
         * @param event 日志现场
         * @return 处理后的输出流
         */
        std::ostream &format(std::ostream &os, LogEvent::ptr &event);

        // region ## Getter
        bool isError() const { return error_; }

        const std::string &getPattern() const { return pattern_; }
        // endregion

    public:
        /**
         * @brief 内部类，需要被继承以实现 format 方法，每一个子类负责输出 pattern 里的一项
         */
        class FormatItem {
        public:
            using ptr = std::shared_ptr<FormatItem>;

            virtual ~FormatItem() = default;

            virtual void format(std::ostream &os, LogEvent::ptr event) = 0;
        };

    private:
        /// 标志成员变量，表示解析过程是否出错
        bool error_;
        /// 日志的打印格式
        std::string pattern_;
        /// 根据 pattern 生成的一组按序的 FormatItem
        std::vector<FormatItem::ptr> items_;
    };

    /**
     * @brief 日志输出地，需要被继承
     */
    class LogAppender {
    public:
        using ptr = std::shared_ptr<LogAppender>;

        /**
         * @brief 构造函数
         * @param formatter 日志格式器
         */
        explicit LogAppender(LogFormatter::prt formatter) : formatter_(std::move(formatter)) {};

        virtual ~LogAppender() = default;

        /**
         * @brief log 打印日志，需要子类实现
         * @param event 日志现场
         */
        virtual void log(LogEvent::ptr event) = 0;

        // region # Getter && Setter
        const LogFormatter::prt &getFormatter() const {
            return formatter_;
        }

        void setFormatter(LogFormatter::prt formatter) {
            formatter_ = std::move(formatter);
        }
        // endregion

    protected:
        /// 自旋锁
        // 一个 LogAppender 可以放到多个 Logger 里面，可能在多个线程同时操作，又因为日志打印非常快，所以自旋锁等待片刻即可，不需要使用互斥锁
        SpinLock mutex_;
        /// 日志格式器
        LogFormatter::prt formatter_;
    };

    /**
     * @brief 标准输出流输出地
     */
    class StdoutLogAppender : public LogAppender {
    public:
        /**
         * @brief 构造函数
         * @note 日志格式器使用默认格式，用户若是不想，则调用 setFormatter 设置
         */
        StdoutLogAppender() : LogAppender(std::make_shared<LogFormatter>()) {};

        /**
         * @brief 重写父类的 log 方法，实现往标准输出打印日志
         * @param event 日志现场
         */
        void log(LogEvent::ptr event) override;
    };

    /**
     * @brief 文件输出地
     */
    class FileLogAppender : public LogAppender {
    public:
        /**
         * @brief 构造函数
         * @param filename 文件名
         * @note 日志格式器使用默认格式，用户若是不想，则调用 setFormatter 设置
         */
        explicit FileLogAppender(std::string filename);

        /**
         * @brief 重写父类的 log 方法，实现往标准输出打印日志
         * @param event 日志现场
         */
        void log(LogEvent::ptr event) override;

        /**
         * @brief 文件超时重开
         * @return 操作是否出错
         */
        bool reopen();

    private:
        /// 保存日志的文件名
        std::string filename_;
        /// 文件输出流
        std::ofstream filestream_;
        /// 文件重开是否出错
        bool reopen_error_;
        /// 上一次打开文件的时间
        uint64_t last_open_time_;
    };

    /**
     * @brief 日志器
     */
    class Logger {
    public:
        using ptr = std::shared_ptr<Logger>;

        /**
         * @brief 构造函数
         * @param logger_name 日志器名称
         * @param logger_level 日志器等级，日志事件要大于等于该等级才会被该日志器打印，默认为 INFO
         */
        explicit Logger(std::string logger_name, LogLevel::Level logger_level = LogLevel::INFO);

        /**
         * @brief 如果日志等级满足，向该日志器包含的所有日志输出地打印该日志事件
         * @param event 日志现场
         */
        void log(const LogEvent::ptr &event);

        // region # Getter && Setter
        void addAppender(const LogAppender::ptr &appender);

        void delAppender(const LogAppender::ptr &appender);

        void clearAppender();

        const std::string &getLoggerName() const {
            return logger_name_;
        }

        LogLevel::Level getLoggerLevel() const {
            return logger_level_;
        }

        uint64_t getCreateTime() const {
            return create_time_;
        }

        void setLoggerLevel(LogLevel::Level loggerLevel) {
            logger_level_ = loggerLevel;
        }
        // endregion

    private:
        /// 自旋锁
        SpinLock mutex_;
        /// 日志器名称
        std::string logger_name_;
        /// 日志器日志级别，大于等于这个级别的日志才会输出
        LogLevel::Level logger_level_;
        /// 日志输出地数组
        std::list<LogAppender::ptr> appenders_;
        /// 该日志器的创建时间
        uint64_t create_time_;
    };

    /**
     * @brief 日志器管理器
     */
    class LoggerManager {
    public:
        /**
         * @brief 构造函数，主要用来构造 root logger，主日志器默认日志等级为 DEBUG，即什么级别的日志都可以打印，处理 UNKNOWN
         */
        LoggerManager();

        /**
         * @brief 获得主日志器
         * @return 主日志器
         */
        Logger::ptr getRootLogger();

        /**
         * @brief 根据名字获得对应的日志器，如果没有就构造一个
         * @param name 日志器名字
         * @return 对应的日志器
         */
        Logger::ptr getLogger(const std::string &name);

    private:
        /// 自旋锁
        SpinLock mutex_;
        /// 主日志器
        Logger::ptr root_logger_;
        /// 其他日志器
        std::unordered_map<std::string, Logger::ptr> loggers_;
    };

    using LoggerMgr = Singleton<LoggerManager>;

    /**
     * @brief 日志器和日志事件包装类
     */
    class LogEventWrapper {
    public:
        /**
         * @brief 构造函数
         * @param logger 日志器
         * @param event 日志事件，通过宏定义快速构造
         */
        LogEventWrapper(Logger::ptr logger, LogEvent::ptr event)
                : logger_(std::move(logger)), event_(std::move(event)) {
        }

        /**
         * @brief 析构函数，在析构时完成日志的打印
         */
        ~LogEventWrapper() {
            logger_->log(event_);
        }

        /**
         * @brief 获取日志事件，因为日志事件是通过宏快速构造的，通过该方法获该日志事件，进一步获取字符串流或者调用 fmt Print
         * @return 日志事件
         */
        const LogEvent::ptr &getLogEvent() const { return event_; }

    private:
        /// 日志器
        Logger::ptr logger_;
        /// 日志事件
        LogEvent::ptr event_;
    };

}

#endif //LUWU_LOGGER_H
