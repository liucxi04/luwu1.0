//
// Created by liucxi on 2022/11/1.
//

#ifndef LUWU_UTIL_H
#define LUWU_UTIL_H

#include <cstdint>
#include <string>

namespace luwu {

    uint32_t getThreadId();

    uint32_t getFiberId();

    uint64_t getElapseMs();

    std::string getThreadName();

    void setThreadName(const std::string &name);

}

#endif //LUWU_UTIL_H
