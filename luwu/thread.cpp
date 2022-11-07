//
// Created by liucxi on 2022/11/6.
//

#include "thread.h"
#include "logger.h"
#include "utils/util.h"

namespace luwu {

    // 线程局部变量，保存当前正在执行的线程
    static thread_local Thread *t_thread;

    Thread::Thread(std::string name, std::function<void()> func)
        : id_(-1), name_(std::move(name)), thread_func_(std::move(func)) {
        // pthread_create 执行之后，Thread::MainThread 就已经开始运行
        int rt = pthread_create(&thread_, nullptr, &Thread::MainThread, this);
        if (rt) {
            LUWU_LOG_ERROR(LUWU_LOG_ROOT()) << "pthread_create error";
        }
        // 如果当前函数先退出，那么一些变量的初始化还没有完成，会影响用户获取线程名和线程 id，所以需要等待信号量
        sem_.wait();
    }

    Thread::~Thread() {
        // // 如果主线程结束时子线程还没结束，那么分离主线程和子线程
        if (thread_) {
            int rt = pthread_detach(thread_);
            if (rt) {
                LUWU_LOG_ERROR(LUWU_LOG_ROOT()) << "pthread_detach error";
            }
        }
    }

    void Thread::join() {
        if (thread_) {
            int rt = pthread_join(thread_, nullptr);
            if (rt) {
                LUWU_LOG_ERROR(LUWU_LOG_ROOT()) << "pthread_join error";
            }
        }
        thread_ = 0;
    }

    Thread *Thread::GetThis() {
        return t_thread;
    }

    void *Thread::MainThread(void *arg) {
        auto *thread = static_cast<Thread *>(arg);
        // 设置当前线程的 id
        thread->id_ = getThreadId();
        // 设置当前线程的名字
        setThreadName(thread->getName());

        t_thread = thread;
        // swap 交换空间之后，两个线程的执行就不会相互影响了
        std::function<void()> callback;
        callback.swap(t_thread->thread_func_);

        // 所有变量都初始化完成，可以给构造函数一个信号了
        t_thread->sem_.notify();
        callback();
        return nullptr;
    }
}