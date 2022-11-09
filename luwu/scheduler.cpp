//
// Created by liucxi on 2022/11/7.
//

#include "scheduler.h"
#include "utils/asserts.h"

namespace luwu {
    // 当前线程所属的调度器，同一个调度器下的所有线程共享同一个实例
    static thread_local Scheduler *t_scheduler = nullptr;
    // 当前线程的调度协程，调度器所在线程的调度协程不是主协程，其余线程的调度协程为主协程
    static thread_local Fiber *t_scheduler_fiber = nullptr;

    // region # Scheduler::Scheduler()
    Scheduler::Scheduler(std::string name, uint32_t thread_num, bool use_caller)
        : name_(std::move(name)), stopping_(false), thread_num_(thread_num)
        , active_thread_num_(0), idle_thread_num_(0)
        , use_caller_(use_caller), caller_tid_(-1) {
        setThreadName(name_);
        // 初始化主线程的主协程，即调度器所在的协程
        Fiber::InitMainFiber();

        if (use_caller) {
            LUWU_ASSERT(GetThis() == nullptr);
            // 再初始化一个主线程的调度协程，使用 Scheduler::run 作为入口函数
            // 调度器所在线程的主协程和调度协程不是同一个
            // 该调度协程会在调度器退出前执行
            caller_fiber_.reset(new Fiber(std::bind(&Scheduler::run, this), false));
            caller_tid_ = getThreadId();

            // 调度器所在线程，即主线程的线程局部变量初始化
            SetThis(this);
            t_scheduler_fiber = caller_fiber_.get();
        }
    }
    // endregion

    Scheduler::~Scheduler() {
        LUWU_ASSERT(stopping_ == true);
        if (GetThis() == this) {
            // 析构函数会在主线程的主协程内被调用
            // GetThis() != this 的情况只有 GetThis() == nullptr，即没有给它赋值
            // 可能出现的时机为 use_caller_ = false
            t_scheduler = nullptr;
        }
    }

    void Scheduler::start() {
        Mutex::Lock lock(mutex_);

        LUWU_ASSERT(threads_.empty());
        threads_.resize(thread_num_);
        // 创建对应数量的子线程，子线程的入口函数也是子线程主协程的入口函数
        for (int i = 0; i < thread_num_; ++i) {
            threads_[i].reset(
                    new Thread(name_ + "_" + std::to_string(i), std::bind(&Scheduler::run, this))
            );
        }
    }

    void Scheduler::stop() {
        stopping_ = true;

        // Scheduler::stop() 会在主线程的主协程内被调用
        // use_caller_ = true，GetThis() 才有值，否则其为 nullptr
        if (use_caller_) {
            LUWU_ASSERT(GetThis() == this);
        } else {
            LUWU_ASSERT(GetThis() != this);
        }

        // 通知所有子线程的调度协程退出调度
        for (int i = 0; i < thread_num_; ++i) {
            tickle();
        }

        // 通知主线程的调度协程退出调度
        if (caller_fiber_) {
            tickle();
        }

        // 在 use_caller_ = true 的情况下，调度器退出时要调用 caller_fiber_
        if (caller_fiber_) {
            caller_fiber_->resume();
        }

        std::vector<Thread::ptr> threads;
        {
            Mutex::Lock lock(mutex_);
            threads.swap(threads_);
        }
        // 等待所有任务都调度完成后才可以退出
        for (auto &t : threads) {
            t->join();
        }
    }

    bool Scheduler::stopping() {
        Mutex::Lock lock(mutex_);
        // 所有任务都执行结束才可以停止调度器
        return stopping_ && tasks_.empty() && active_thread_num_ == 0;
    }

    void Scheduler::run() {
        // TODO hook
        // set_hook_enable();
        SetThis(this);
        // 当前线程不是调度器所在的线程，那么就是子线程，所以需要初始化主协程和调度协程，这两个协程是同一个
        if (getThreadId() != caller_tid_) {
            Fiber::InitMainFiber();
            t_scheduler_fiber = Fiber::GetThis();
        }

        // 空闲协程
        Fiber::ptr idle_fiber(new Fiber(std::bind(&Scheduler::idle, this)));

        static thread_local SchedulerTask task;
        while (true) {
            task.reset();
            // 是否需要 tickle 其他协程进行任务调度
            bool tickle_me = false;
            {
                Mutex::Lock lock(mutex_);
                auto it = tasks_.begin();
                while (it != tasks_.end()) {
                    // 指定了某个线程进行调度，但是不是本线程，所以需要通知其他线程
                    if (it->tid_ != -1 && it->tid_ != getThreadId()) {
                        ++it;
                        tickle_me = true;
                        continue;
                    }

                    // TODO hook

                    // 找到了一个合法的任务
                    LUWU_ASSERT(it->fiber_ || it->func_);
                    task = *it;
                    tasks_.erase(it);
                    ++active_thread_num_;
                    break;
                }
                // 当前调度协程拿走一个任务后，还有剩余任务，也需要通知其他线程继续调度
                tickle_me |= (it != tasks_.end());
            }

            if (tickle_me) {
                tickle();
            }

            if (task.fiber_) {                                          // 协程直接调度
                task.fiber_->resume();
                --active_thread_num_;
            } else if (task.func_) {
                Fiber::ptr func_fiber(new Fiber(task.func_));   // 函数封装成协程再调度
                func_fiber->resume();
                --active_thread_num_;
            } else {                                                    // 没有任务了，进入到 idle 协程
                if (idle_fiber->getState() == Fiber::TERM) {            // idle 协程在满足退出条件后会退出，执行状态变成 TERM
                    break;
                }
                ++idle_thread_num_;
                idle_fiber->resume();
                --idle_thread_num_;
            }
        }
    }

    void Scheduler::idle() {
        // 此处的 idle 协程什么都不做，进来就退出
        // 调度器可以停止时，就会退出 while 循环，idle 协程就执行结束了，协程状态变味了 TERM
        // 之后 Scheduler::run() 的 while(true) 就会结束
        while (!stopping()) {
            Fiber::GetThis()->yield();
        }
    }

    void Scheduler::tickle() {

    }

    Scheduler *Scheduler::GetThis() {
        return t_scheduler;
    }

    void Scheduler::SetThis(Scheduler *scheduler) {
        t_scheduler = scheduler;
    }

    Fiber *Scheduler::GetSchedulerFiber() {
        return t_scheduler_fiber;
    }
}