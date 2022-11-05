//
// Created by liucxi on 2022/11/1.
//

#ifndef LUWU_SINGLETON_H
#define LUWU_SINGLETON_H

#include "noncopyable.h"

namespace luwu {
    /**
     * @brief 单例模式模板类
     * @tparam T 类型
     */
    template<typename T>
    class Singleton : NonCopyable {
    public:
        static T &GetInstance() {
            static T instance_;
            return instance_;
        }
    };
}

#endif //LUWU_SINGLETON_H
