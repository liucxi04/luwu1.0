//
// Created by liucxi on 2022/11/4.
//

#ifndef LUWU_ASSERTS_H
#define LUWU_ASSERTS_H

#include <cassert>
#include "../logger.h"
#include "util.h"

#define LUWU_ASSERT(x) \
    if (!(x)) {        \
        LUWU_LOG_ERROR(LUWU_LOG_ROOT()) << \
                "LUWU_ASSERT:" << #x << "\nbacktrace:\n" << luwu::backtraceToString(64, 2, "    "); \
        assert(x);                   \
    }

#define LUWU_ASSERT2(x, w) \
    if (!(x)) {        \
        LUWU_LOG_ERROR(LUWU_LOG_ROOT()) << \
                "LUWU_ASSERT:" << #x << "\n" << w << "\nbacktrace:\n" << luwu::backtraceToString(64, 2, "    "); \
        assert(x);                   \
    }

#endif //LUWU_ASSERTS_H
