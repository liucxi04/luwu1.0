//
// Created by liucxi on 2022/11/7.
//

#ifndef LUWU_SCHEDULER_H
#define LUWU_SCHEDULER_H

#include <list>
#include <vector>
#include <atomic>
#include "thread.h"
#include "fiber.h"
#include "utils/mutex.h"
#include "utils/noncopyable.h"

namespace luwu {
    /**
     * @brief 协程调度器，可以被继承
     */
    class Scheduler : NonCopyable {
    public:
        using ptr = std::shared_ptr<Scheduler>;

        /**
         * @brief 构造函数
         * @param name 调度器名称
         * @param thread_num 子线程数量
         * @param use_caller 调度器所在线程是否作为调度线程
         */
        explicit Scheduler(std::string name, uint32_t thread_num = 0, bool use_caller = true);

        /**
         * @brief 析构函数
         */
        virtual ~Scheduler();

        /**
         * @brief 启动调度器
         */
        void start();

        /**
         * @brief 停止调度器
         * @details 等所有调度任务都执行完了再返回
         */
        void stop();

        /**
         * @brief 向调度器添加调度任务
         * @tparam Task 调度任务类型，可以是协程或者函数
         * @param t 协程或者函数
         * @param tid 指定在某一个线程执行
         */
        template<typename Task>
        void addTask(Task t, uint32_t tid = -1) {
            bool tickle_me;
            {
                Mutex::Lock lock(mutex_);
                tickle_me = tasks_.empty();
                SchedulerTask task(t, tid);
                if (task.fiber_ || task.func_) {
                    tasks_.push_back(task);
                }
            }
            if (tickle_me) {
                tickle();
            }
        }

    protected:
        /**
         * @brief 调度器是否可以停止
         * @return 是否可以停止
         */
        virtual bool stopping();

        /**
         * @brief 线程入口函数，也是线程主协程的入口函数，也称作调度协程
         */
        virtual void run();

        /**
         * @brief 空闲协程，在线程没有调度任务时转到该协程执行
         */
        virtual void idle();

        /**
         * @brief 通知协程调度器有调度任务需要执行了
         */
        virtual void tickle();

    public:
        /**
         * @brief 获取当前线程所属的调度器
         * @return 当前线程所属的调度器
         */
        static Scheduler *GetThis();

        /**
         * @brief 设置当前线程所属的调度器
         * @param scheduler 当前线程所属的调度器
         */
        static void SetThis(Scheduler *scheduler);

        /**
         * @brief 获得当前线程的调度协程
         * @return 当前线程的调度协程
         */
        static Fiber *GetSchedulerFiber();

    private:
        /**
         * @brief 调度任务，可以是协程或者函数
         */
        struct SchedulerTask {
            /// 协程
            Fiber::ptr fiber_;
            /// 函数
            std::function<void()> func_;
            /// 指定在某个线程运行
            uint32_t tid_;

            SchedulerTask() : fiber_(nullptr), func_(nullptr), tid_(-1) {}

            explicit SchedulerTask(Fiber::ptr fiber, uint32_t tid = -1)
                : fiber_(std::move(fiber)), func_(nullptr), tid_(tid) {
            }

            explicit SchedulerTask(std::function<void()> func, uint32_t tid = -1)
                : fiber_(nullptr), func_(std::move(func)), tid_(tid) {
            }

            void reset() {
                fiber_ = nullptr;
                func_ = nullptr;
                tid_ = -1;
            }
        };

    private:
        Mutex mutex_;
        /// 调度器名称
        std::string name_;
        /// 调度器是否正在停止
        bool stopping_;

        /// 调度器需要调度的任务队列
        std::list<SchedulerTask> tasks_;

        /// 线程池
        std::vector<Thread::ptr> threads_;
        /// 除调度器所在线程之外的线程数量，子线程的数量，线程池的大小
        uint32_t thread_num_;
        /// 活跃线程数量
        std::atomic_uint32_t active_thread_num_;
        /// 空闲线程数量
        std::atomic_uint32_t idle_thread_num_;

        /// 调度器所在的线程是否参数调度
        bool use_caller_;
        /// use_caller_ 为 true 时，调度器所在线程的调度协程，和线程主协程不是同一个
        Fiber::ptr caller_fiber_;
        /// use_caller_ 为 true 时，调度器所在线程的线程 id
        uint32_t caller_tid_;
    };
}
#endif //LUWU_SCHEDULER_H
