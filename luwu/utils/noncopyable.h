//
// Created by liucxi on 2022/11/1.
//

#ifndef LUWU_NONCOPYABLE_H
#define LUWU_NONCOPYABLE_H

namespace luwu {
    class NonCopyable {
    public:
        NonCopyable(const NonCopyable &) = delete;

        NonCopyable &operator=(const NonCopyable &) = delete;

        NonCopyable(NonCopyable &&) = delete;

        NonCopyable &operator=(NonCopyable &&) = delete;

    protected:
        NonCopyable() = default;

        ~NonCopyable() = default;
    };
}

#endif //LUWU_NONCOPYABLE_H
