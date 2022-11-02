//
// Created by liucxi on 2022/11/1.
//

#ifndef LUWU_MUTEX_H
#define LUWU_MUTEX_H

#include <pthread.h>
#include "noncopyable.h"

namespace luwu {

    /**
     * @brief 局部锁模板类，可基于此泛化出自旋锁、互斥锁等
     */
    template<typename T>
    class ScopedLockImpl {
    public:
        explicit ScopedLockImpl(T &mutex) : mutex_(mutex), locked_(false) {
            mutex_.lock();
            locked_ = true;
        }

        ~ScopedLockImpl() {
            unlock();
        }

        void lock() {
            if (!locked_) {
                mutex_.lock();
                locked_ = true;
            }
        }

        void unlock() {
            if (locked_) {
                mutex_.unlock();
                locked_ = false;
            }
        }

    private:
        T &mutex_;
        bool locked_;
    };

    /**
     * @brief 自锁封装
     */
    class SpinLock : NonCopyable {
    public:
        using Lock = ScopedLockImpl<SpinLock>;

        SpinLock() {
            pthread_spin_init(&mutex_, 0);
        }

        ~SpinLock() {
            pthread_spin_destroy(&mutex_);
        }

        void lock() {
            pthread_spin_lock(&mutex_);
        }

        void unlock() {
            pthread_spin_unlock(&mutex_);
        }
    private:
        pthread_spinlock_t mutex_{};
    };

    /**
     * @brief 互斥锁，互斥量
     */
    class Mutex : NonCopyable {
    public:
        Mutex() {
            pthread_mutex_init(&mutex_, nullptr);
        }

        ~Mutex() {
            pthread_mutex_destroy(&mutex_);
        }

        void lock() {
            pthread_mutex_lock(&mutex_);
        }

        void unlock() {
            pthread_mutex_unlock(&mutex_);
        }
    private:
        pthread_mutex_t mutex_{};
    };
}

#endif //LUWU_MUTEX_H
