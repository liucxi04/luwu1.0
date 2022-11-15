//
// Created by liucxi on 2022/11/11.
//

#ifndef LUWU_REACTOR_H
#define LUWU_REACTOR_H

#include "utils/noncopyable.h"
#include "scheduler.h"
#include "clock.h"

namespace luwu {

    /**
     * @brief IO 事件，与 epoll 对事件的定义相同，暂时只关心读写事件
     */
    struct ReactorEvent {
        enum Event {
            NONE = 0x00,
            READ = 0x01,
            WRITE = 0x04,
        };
    };

    /**
     * @brief socket fd 上下文，fd - 事件 -回调三元组
     */
    struct Channel : NonCopyable {
        /**
         * @brief 事件上下文，包含执行回调函数的调度器和需要执行的回调
         */
        struct EventCallback {
            Scheduler *scheduler_ = nullptr;
            Fiber::ptr fiber_;
            std::function<void()> func_;
        };

        /**
         * @brief 构造函数
         * @param fd socket fd
         * @param event 感兴趣的事件
         */
        explicit Channel(int fd, ReactorEvent::Event event = ReactorEvent::NONE);

        /**
         * @brief 获取对应事件的回调
         * @param event 事件
         * @return 返回回调的引用，可以进行修改
         */
        EventCallback &getEventCallback(ReactorEvent::Event event);

        /**
         * @brief 重置回调
         * @param event_callback 需要被重置的回调
         */
        static void resetEventCallback(EventCallback &event_callback);

        /**
         * @brief 触发对应事件的回调
         * @param event 事件
         * @details 触发回调只是将对应的回调函数添加到调度器中，而不是立即执行
         */
        void triggerEvent(ReactorEvent::Event event);

        /// socket 描述符
        int fd_;
        /// 感兴趣的事件，多个事件用 | 连接
        ReactorEvent::Event event_;
        /// 读事件回调
        EventCallback read_;
        /// 写事件回调
        EventCallback write_;
        Mutex mutex_;
    };

    // TODO Poller 重构

    /**
     * @brief 反应堆模型，事件循环的封装
     */
    class Reactor : public Scheduler, public ClockManager {
    public:
        using ptr = std::shared_ptr<Reactor>;

    public:
        /**
         * @brief 构造函数
         * @param name 调度器名称
         * @param thread_num 子线程数量
         * @param use_caller 调度器所在线程是否作为调度线程
         */
        explicit Reactor(std::string name, uint32_t thread_num = 0, bool use_caller = true);

        /**
         * @brief 析构函数
         */
        ~Reactor() override;

        /**
         * @brief 将 fd 的 event 事件添加到 epoll 中
         * @param fd socket 描述符
         * @param event 感兴趣的事件
         * @param cb 事件对应的回调
         * @return 操作是否成功
         */
        bool addEvent(int fd, ReactorEvent::Event event, const std::function<void()>& cb = nullptr);

        /**
         * @brief 将 fd 的 event 事件从 epoll 中删除
         * @param fd socket 描述符
         * @param event 感不兴趣的事件
         * @return 操作是否成功
         */
        bool delEvent(int fd, ReactorEvent::Event event);

        /**
         * @brief 获取当前线程的反应堆模型
         * @return 当前线程的反应堆模型
         */
        static Reactor *GetThis();

    private:
        /**
         * @brief 反应堆是否可以停止
         * @return 是否可以停止
         */
        bool stopping() override;

        /**
         * @brief 空闲协程，在线程没有调度任务时转到该协程执行
         */
        void idle() override;

        /**
         * @brief 通知协程调度器有调度任务需要执行了
         */
        void tickle() override;

        /**
         * @brief 当插入一个定时器到堆顶时需要执行的操作
         */
        void onClockInsertAtFront() override;

        /**
         * @brief 调整 std::vector<Channel *> 的大小
         * @param size 目标大小
         */
        void channelResize(size_t size);

    private:
        /// epoll 描述符
        int epoll_fd_;
        /// event fd，用于唤醒 epoll_wait
        int wakeup_fd_;
        /// epoll 所管理的所有 socket fd
        std::vector<Channel *> channels_;
        RWMutex mutex_;
    };
}
#endif //LUWU_REACTOR_H
