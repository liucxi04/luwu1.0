//
// Created by liucxi on 2022/11/1.
//

#ifndef LUWU_UTIL_H
#define LUWU_UTIL_H

#include <cstdint>
#include <vector>
#include <string>

namespace luwu {

    /**
     * @brief 获得当前线程的线程 id
     * @return 线程 id
     */
    uint32_t getThreadId();

    /**
     * @brief 获得当前协程的协程 id
     * @return 线程 id
     */
    uint32_t getFiberId();

    /**
     * @brief 获得系统从开始运行到现在过去的时间
     * @return 时间
     */
    uint64_t getElapseMs();

    /**
     * @brief 获得当前线程的线程名
     * @return 线程名
     */
    std::string getThreadName();

    /**
     * @brief 设置当前线程的线程名
     * @param name 线程名
     */
    void setThreadName(const std::string &name);


    uint64_t getCurrentTime();

    void backtrace(std::vector<std::string> &bt, int size, int skip);

    std::string backtraceToString(int size, int skip, const std::string &prefix = "");
}

#endif //LUWU_UTIL_H
