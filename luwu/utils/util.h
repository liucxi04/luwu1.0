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

    /**
     * @brief 获取当前时间
     * @return 系统当前时间
     */
    uint64_t getCurrentTime();

    /**
     * @brief 获取程序调用栈
     * @param bt 保存栈信息
     * @param size 栈深度
     * @param skip 忽略前 size 层信息
     */
    void backtrace(std::vector<std::string> &bt, int size, int skip);

    /**
     * @brief 将程序调用栈格式化为字符串
     * @param size 栈深度
     * @param skip 忽略前 size 层信息
     * @param prefix 前缀信息
     * @return 字符串
     */
    std::string backtraceToString(int size, int skip, const std::string &prefix = "");

    /**
     * @brief 格式化时间表示
     * @param ts time_t 类型的时间
     * @param format 目标格式
     * @return 字符串
     */
    std::string formatTime(time_t ts = time(nullptr), const std::string &format = "%Y-%m-%d %H:%M:%S");

    // TODO 理解
    /**
     * @brief url 编码
     * @param str 编码前的字符串
     * @param space_as_plus 是否将空格编码为 +，为 false 则编码为 %20
     * @return 编码后的字符串
     */
    std::string urlEncode(const std::string &str, bool space_as_plus = true);

    /**
     * @brief url 解码
     * @param str 解码前的字符串
     * @param space_as_plus 是否将 + 解码为空格
     * @return 解码后的字符串
     */
    std::string urlDecode(const std::string &str, bool space_as_plus = true);

    /**
     * @brief 取出字符串首尾的指定字符串
     * @param str 处理前的字符串
     * @param delimit 待移除的字符串
     * @return 处理后的字符串
     */
    std::string trim(const std::string &str, const std::string &delimit = " \t\r\n");

    /**
     * @brief base64 编码算法
     * @param data 需要处理的数据
     * @param len 数据长度
     * @return 处理后的字符串
     */
    std::string base64encode(const void* data, size_t len);

    /**
     * @brief sha1 散列算法
     * @param data 需要处理的数据
     * @param len 数据长度
     * @return 处理后的字符串
     */
    std::string sha1sum(const void *data, size_t len);
}

#endif //LUWU_UTIL_H
