//
// Created by liucxi on 2022/11/1.
//

#include "util.h"
#include <unistd.h>
#include <sys/syscall.h>
#include <execinfo.h>
#include <sstream>
#include <iostream>

namespace luwu {
    uint32_t getThreadId() {
        return syscall(SYS_gettid);
    }

    uint32_t getFiberId() {
        // TODO Fiber::GetId()
        return 0;
    }

    // 将 Logger 定义中的 create_time_ 初始化由 time(nullptr) 改为 create_time_(getElapseMs()) 基本满足要求
    // 后续有更好的办法 ？ std::chrono ？
    uint64_t getElapseMs() {
        struct timespec ts{};
        clock_gettime(CLOCK_MONOTONIC_RAW, &ts);            // 系统开始运行到现在的时间
        return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
    }

    std::string getThreadName() {
        // 系统调用要求不能超过 16 字节
        char thread_name[16];
        pthread_getname_np(pthread_self(), thread_name, 16);
        return thread_name;
    }

    void setThreadName(const std::string &name) {
        pthread_setname_np(pthread_self(), name.substr(0, 15).c_str());
    }

    void backtrace(std::vector<std::string> &bt, int size, int skip) {
        void **array = (void **) ::malloc(sizeof(void *) * size);
        int s = ::backtrace(array, size);

        char **strings = ::backtrace_symbols(array, size);
        if (strings == nullptr) {
            std::cerr << "backtrace_symbols error" << std::endl;
        }

        for (int i = skip; i < s; ++i) {
            bt.emplace_back(strings[i]);
        }
        ::free(array);
        ::free(strings);
    }

    std::string backtraceToString(int size, int skip, const std::string &prefix) {
        std::vector<std::string> bt;
        backtrace(bt, size, skip);

        std::stringstream ss;
        for (const auto &i: bt) {
            ss << prefix << " " << i << std::endl;
        }
        return ss.str();
    }
}
