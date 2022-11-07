//
// Created by liucxi on 2022/11/3.
//

#include "fiber.h"
#include <atomic>
#include <utility>
#include "utils/asserts.h"

namespace luwu {

    // 全局静态变量，这里的全局指的是进程内，所以有多线程同时操作的可能

    // 只增不减，用来标识唯一的协程
    static std::atomic<uint32_t> s_fiber_id{0};
    // 可增可减，用来记录当前正在运行的线程数量（进程级别）
    static std::atomic<uint32_t> s_fiber_num{0};

    // 线程局部变量，不同线程有不同的对象

    // 当前线程的主协程，智能指针
    static thread_local Fiber::ptr t_main_fiber = nullptr;
    /**
     * @brief 当前线程正在执行的协程，裸指针
     * @note 为什么不使用智能指针？是因为有可能出现循环引用的问题。
     * 但更现实的问题是在实现过程中，无论是在 SetThis() 的参数中使用智能指针还是裸指针都会有问题。
     * 使用智能指针，在函数内赋值没问题，但是由于 SetThis() 会在构造函数内被调用，此时对象尚未构造完成，shared_from_this() 出错
     * 使用裸指针，在未构造完成的对象内使用 this 指针没问题，可以正常将值传递给 SetThis()，但是将裸指针赋值给智能指针会有问题。
     * 加上 t_thread_fiber 指示当前正在运行的协程，会不断改变指向，使用智能指针还有引用计数等的问题要分析，综上，使用裸指针实现简单。
     * 但是用户在构造 Fiber 对象时可能会使用 Fiber::ptr，所以还会出现裸指针与智能指针混用的问题，无论怎样都没有想到一个完美的解决方案。
     */
    static thread_local Fiber *t_thread_fiber = nullptr;
    // 默认的协程栈空间大小
    static const uint32_t stack_size = 128 * 1024;

    Fiber::Fiber()
        : id_(s_fiber_id++), state_(RUNNING), stack_size_(0), run_in_scheduler_(false) {
        ++s_fiber_num;
        SetThis(this);      // 创建主协程时线程没有其他协程，当前正在运行的协程，由于 this 指针的缘故，必须在这里设置
        if (::getcontext(&context_)) {
            LUWU_ASSERT2(false, "getcontext");
        }
    }

    Fiber::Fiber(fiber_func func, bool run_in_scheduler)
        : id_(s_fiber_id++), state_(READY), func_(std::move(func))
        , stack_size_(stack_size), run_in_scheduler_(run_in_scheduler) {
        ++s_fiber_num;

        // 创建写成之前要看一下有没有主协程，如果没有要先创建主协程
        if (!t_main_fiber) {
            t_main_fiber = Fiber::ptr(new Fiber);           // 创建主协程
            LUWU_ASSERT(t_main_fiber);                         // 现在有主协程了
            LUWU_ASSERT(t_thread_fiber == t_main_fiber.get()); // 当前正在运行的协程就是主协程
        }

        stack_ = ::malloc(stack_size_);
        if (::getcontext(&context_)) {
            LUWU_ASSERT2(false, "getcontext");
        }
        // 当前上下文结束之后运行的上下文为空，那么在本协程运行结束时必须要调用 setcontext 或 swapcontext 以重新指定一个有效的上下文，
        // 否则程序就跑飞了，在代码中体现为 Fiber::MainFunc 的最后 yield 一次，在 yield 中恢复了主协程的运行
        context_.uc_link = nullptr;
        context_.uc_stack.ss_sp = stack_;
        context_.uc_stack.ss_size = stack_size_;
        makecontext(&context_, &Fiber::MainFunc, 0);    // 设置协程入口函数
    }

    Fiber::~Fiber() {
        --s_fiber_num;
        if (stack_) {                       // 有栈空间，说明是普通协程
            LUWU_ASSERT(state_ == TERM);
            ::free(stack_);
        } else {                            // 没有栈空间，说明是主协程
            LUWU_ASSERT(state_ == RUNNING);
            LUWU_ASSERT(!func_);
            if (t_thread_fiber == this) {
                SetThis(nullptr);
            }
        }
    }

    void Fiber::reset(fiber_func func) {
        LUWU_ASSERT(stack_);
        LUWU_ASSERT(state_ == TERM);

        state_ = READY;
        func_ = std::move(func);

        if (::getcontext(&context_)) {
            LUWU_ASSERT2(false, "getcontext");
        }
        context_.uc_link = nullptr;
        context_.uc_stack.ss_size = stack_size_;
        context_.uc_stack.ss_sp = stack_;
        makecontext(&context_, &Fiber::MainFunc, 0);
    }

    void Fiber::yield() {
        LUWU_ASSERT(state_ == RUNNING || state_ == TERM);

        SetThis(t_main_fiber.get());                    // 当前协程退出执行，要将线程正在执行的协程修改为主协程
        if (state_ == RUNNING) {
            state_ = READY;
        }
        if (run_in_scheduler_) {

        } else {
            // 当前协程栈空间保存在第一个参数里，从第二个参数读出协程占空间恢复执行
            if (swapcontext(&context_, &(t_main_fiber->context_))) {
                LUWU_ASSERT2(false, "swapcontext error");
            }
        }
    }

    void Fiber::resume() {
        LUWU_ASSERT(state_ == READY);

        SetThis(this);                                  // 当前协程需要恢复执行，要将线程正在执行的协程修改为当前协程
        state_ = RUNNING;
        if (run_in_scheduler_) {

        } else {
            // 当前协程栈空间保存在第一个参数里，从第二个参数读出协程占空间恢复执行
            if (swapcontext(&(t_main_fiber->context_), &context_)) {
                LUWU_ASSERT2(false, "swapcontext error");
            }
        }
    }

    void Fiber::SetThis(Fiber *fiber) {
        t_thread_fiber = fiber;
    }

    Fiber *Fiber::GetThis() {
        return t_thread_fiber;
    }

    uint32_t Fiber::GetFiberId() {
        return t_thread_fiber->getId();
    }

    uint32_t Fiber::TotalFibers() {
        return s_fiber_num;
    }

    void Fiber::MainFunc() {
        auto cur = GetThis();
        LUWU_ASSERT(cur);

        cur->func_();
        cur->func_ = nullptr;
        cur->state_ = TERM;
        cur->yield();                   // 协程运行结束时必须要 yield 一次，以返回主协程
    }
}
