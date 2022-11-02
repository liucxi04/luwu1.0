//
// Created by liucxi on 2022/11/1.
//

#ifndef LUWU_SINGLETON_H
#define LUWU_SINGLETON_H

namespace luwu {
    template<typename T>
    class Singleton {
    public:
        static T &GetInstance() {
            static T instance_;
            return instance_;
        }
    };
}

#endif //LUWU_SINGLETON_H
