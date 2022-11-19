//
// Created by liucxi on 2022/11/3.
//

#ifndef LUWU_FIBER_H
#define LUWU_FIBER_H

#include <ucontext.h>
#include <memory>
#include <functional>
#include "utils/noncopyable.h"

namespace luwu {
    /**
     * @brief 协程类
     */
    class Fiber : NonCopyable, public std::enable_shared_from_this<Fiber> {
    public:
        using ptr = std::shared_ptr<Fiber>;

        /**
         * @brief 协程内需要执行的任务
         */
        using fiber_func = std::function<void()>;

        /**
         * @brief 协程执行状态，就绪态、运行态、结束态
         */
        enum State {
            READY,
            RUNNING,
            TERM,
        };

        /**
         * @brief 构造函数
         * @param func 协程内需要执行的任务
         * @param run_in_scheduler 本协程是否接受协程调度器调度
         */
        explicit Fiber(fiber_func func, bool run_in_scheduler = true);

        /**
         * @brief 析构函数
         */
        ~Fiber();

        /**
         * @brief 重置协程运行状态，复用协程栈空间
         * @param func 新的协程内需要执行的任务
         */
        void reset(fiber_func func);

        /**
         * @brief 让出该线程的执行权，在子协程被调用
         */
        void yield();

        /**
         * @brief 恢复该线程的执行权，在主协程被调用
         */
        void resume();

        // region # Getter
        uint32_t getId() const {
            return id_;
        }

        State getState() const {
            return state_;
        }
        // endregion

    public:
        static void InitMainFiber();
        /**
         * @brief 将该协程设置为当前正在运行的协程
         * @param fiber 需要被设置的协程
         * @note static 全局只有一个，即进程级别，会根据当前正在执行的线程使用对应的线程局部变量
         */
        static void SetThis(Fiber  *fiber);

        /**
         * @brief 获取当前正在运行的协程
         * @return 当前正在运行的协程
         * @note static 全局只有一个，即进程级别，会根据当前正在执行的线程使用对应的线程局部变量
         * @details 这里需要返回智能指针，因为后续很多模块都是使用的智能指针，若是在这里返回裸指针，让后续模块自行转换为智能指针
         * 会导致引用计数的问题，如果其智能指针的引用计数变为零，但是该协程对象还在运行，此时析构时运行状态的断言会出错，其根本原因在这里
         */
        static Fiber::ptr GetThis();

        /**
         * @brief 获取当前正在运行的协程的 id
         * @return 当前正在运行的协程的 id
         */
        static uint32_t GetFiberId();

        /**
         * @brief 获取当前正在运行的协程数量，该值跨线程
         * @return 当前正在运行的协程数量
         */
        static uint32_t TotalFibers();

        /**
         * @brief 协程入口函数，在本函数内调用构造函数内传入的 func
         */
        static void MainFunc();

    private:
        /**
         * @brief 私有无参构造函数，用来构造线程的第一个协程，即线程对应的主协程
         * @details 主协程与普通协程不同，没有需要执行的函数，没有协程栈空间，创建之后就处于运行态
         */
        Fiber();

    private:
        /// 协程 id
        uint32_t id_;
        /// 协程运行状态
        State state_;
        /// 协程内实际执行的函数
        fiber_func func_;
        /// 协程上下文
        ucontext_t context_{};
        /// 协程栈大小
        uint32_t stack_size_;
        /// 协程栈地址
        void *stack_{};
        /// 是否参与协程调度器调度
        bool run_in_scheduler_;
    };
}

#endif //LUWU_FIBER_H
