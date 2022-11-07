//
// Created by liucxi on 2022/11/1.
//

#ifndef LUWU_MUTEX_H
#define LUWU_MUTEX_H

#include <pthread.h>
#include <semaphore.h>
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
     * @brief 局部读锁模板类
     */
    template<typename T>
    class ReadScopedLockImpl {
    public:
        explicit ReadScopedLockImpl(T &mutex) : mutex_(mutex), locked_(false) {
            mutex_.rdlock();
            locked_ = true;
        }

        ~ReadScopedLockImpl() {
            unlock();
        }

        void lock() {
            if (!locked_) {
                mutex_.rdlock();
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
     * @brief 局部读锁模板类
     */
    template<typename T>
    class WriteScopedLockImpl {
    public:
        explicit WriteScopedLockImpl(T &mutex) : mutex_(mutex), locked_(false) {
            mutex_.wrlock();
            locked_ = true;
        }

        ~WriteScopedLockImpl() {
            unlock();
        }

        void lock() {
            if (!locked_) {
                mutex_.wrlock();
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
     * @brief 信号量
     */
    class Semaphore : NonCopyable {
    public:
        explicit Semaphore(uint32_t count = 0) {
            sem_init(&sem_, 0, count);
        }

        ~Semaphore() {
            sem_destroy(&sem_);
        }

        void notify() {
            sem_post(&sem_);
        }

        void wait() {
            sem_wait(&sem_);
        }

    private:
        sem_t sem_{};
    };

    /**
     * @brief 自旋锁
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
     * @brief  读写锁
     */
    class RWMutex : NonCopyable {
    public:
        using ReadLock = ReadScopedLockImpl<RWMutex>;

        using WriteLock = WriteScopedLockImpl<RWMutex>;

        RWMutex() {
            pthread_rwlock_init(&mutex_, nullptr);
        }

        ~RWMutex() {
            pthread_rwlock_destroy(&mutex_);
        }

        void rdlock() {
            pthread_rwlock_rdlock(&mutex_);
        }

        void wrlock() {
            pthread_rwlock_wrlock(&mutex_);
        }

        void unlock() {
            pthread_rwlock_unlock(&mutex_);
        }

    private:
        pthread_rwlock_t mutex_{};
    };

    /**
     * @brief 互斥锁，互斥量
     */
    class Mutex : NonCopyable {
    public:
        using Lock = ScopedLockImpl<Mutex>;

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
