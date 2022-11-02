//
// Created by liucxi on 2022/11/1.
//

#include "util.h"
#include <unistd.h>
#include <sys/syscall.h>

namespace luwu {
    uint32_t getThreadId() {
        return syscall(SYS_gettid);
    }

    uint32_t getFiberId() {
        // TODO Fiber::GetId()
        return 0;
    }

    // TODO 计算该系统运行了多长时间的逻辑有点问题
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
        pthread_setname_np(pthread_self(), name.substr(0,15).c_str());
    }
}
