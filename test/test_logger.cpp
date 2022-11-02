//
// Created by liucxi on 2022/11/1.
//

#include "logger.h"

using namespace luwu;

int main() {

    LogFormatter::prt formatter1(new LogFormatter("formatter1... [%p] [%c] %m%N"));
    LogFormatter::prt formatter2(new LogFormatter("formatter2... [%p] [%c] %m%N"));

    LogAppender::ptr logAppender1(new StdoutLogAppender());
    LogAppender::ptr logAppender2(new FileLogAppender("./test"));

    logAppender1->setFormatter(formatter1);         // 往标准输出的是格式1
    logAppender2->setFormatter(formatter2);         // 往文件输出的是格式2

    Logger::ptr logger1(new Logger("test_logger_1"));                               // INFO
    Logger::ptr logger2(new Logger("test_logger_2", LogLevel::FATAL));   // FATAL
    Logger::ptr logger3 = LUWU_LOG_NAME("test_logger_3");
    logger3->setLoggerLevel(LogLevel::ERROR);                                                     // ERROR
    Logger::ptr logger4 = LUWU_LOG_ROOT();                                                        // DEBUG

    // 每个 Logger 都往两个日志输出地输出，logger4 是 root_logger，默认有一个标准输出，且输出格式为全量信息，日志等级为 DEBUG
    logger1->addAppender(logAppender1);
    logger1->addAppender(logAppender2);

    logger2->addAppender(logAppender1);
    logger2->addAppender(logAppender2);

    logger3->addAppender(logAppender1);
    logger3->addAppender(logAppender2);

    LUWU_LOG_DEBUG(logger1) << "LUWU_LOG_DEBUG 111";     // 不可打印
    LUWU_LOG_INFO(logger1) << "LUWU_LOG_INFO 111 ";
    LUWU_LOG_FMT_ERROR(logger1, "%s LUWU_LOG_ERROR 111", __FUNCTION__ );
    LUWU_LOG_FMT_FATAL(logger1, "%s LUWU_LOG_FATAL 111", __FUNCTION__ );

    LUWU_LOG_DEBUG(logger2) << "LUWU_LOG_DEBUG 222";// 不可打印
    LUWU_LOG_INFO(logger2) << "LUWU_LOG_INFO 222";// 不可打印
    LUWU_LOG_ERROR(logger2) << "LUWU_LOG_ERROR 222";// 不可打印
    LUWU_LOG_FATAL(logger2) << "LUWU_LOG_FATAL 222";

    LUWU_LOG_DEBUG(logger3) << "LUWU_LOG_DEBUG 333";// 不可打印
    LUWU_LOG_INFO(logger3) << "LUWU_LOG_INFO 333";// 不可打印
    LUWU_LOG_ERROR(logger3) << "LUWU_LOG_ERROR 333";
    LUWU_LOG_FATAL(logger3) << "LUWU_LOG_FATAL 333";

    setThreadName("new_thread");
    LUWU_LOG_DEBUG(logger4) << "LUWU_LOG_DEBUG root";
    LUWU_LOG_INFO(logger4) << "LUWU_LOG_INFO root";
    LUWU_LOG_ERROR(logger4) << "LUWU_LOG_ERROR root";
    LUWU_LOG_FATAL(logger4) << "LUWU_LOG_FATAL root";


    logger1->delAppender(logAppender1);
    logger2->clearAppender();
    return 0;
}