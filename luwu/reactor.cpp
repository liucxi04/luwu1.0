//
// Created by liucxi on 2022/11/11.
//

#include "reactor.h"

#include <utility>
#include <unistd.h>
#include <cstring>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include "logger.h"
#include "utils/asserts.h"

namespace luwu {

    Channel::Channel(int fd, ReactorEvent::Event event) : fd_(fd), event_(event) {}

    Channel::EventCallback &Channel::getEventCallback(ReactorEvent::Event event) {
        switch (event) {
            case ReactorEvent::READ:
                return read_;
            case ReactorEvent::WRITE:
                return write_;
            default:
                LUWU_ASSERT2(false, "getEventCallback")
        }
    }

    void Channel::resetEventCallback(Channel::EventCallback &event_callback) {
        event_callback.scheduler_ = nullptr;
        event_callback.fiber_.reset();
        event_callback.func_ = nullptr;
    }

    void Channel::triggerEvent(ReactorEvent::Event event) {
        LUWU_ASSERT(event_ & event);

        EventCallback &callback = getEventCallback(event);
        if (callback.fiber_) {
            callback.scheduler_->addTask(callback.fiber_->shared_from_this());
        } else {
            callback.scheduler_->addTask(callback.func_);
        }
    }

    // region # Reactor::Reactor()
    Reactor::Reactor(std::string name, uint32_t thread_num, bool use_caller)
            : Scheduler(std::move(name), thread_num, use_caller), epoll_fd_(::epoll_create1(EPOLL_CLOEXEC)),
              wakeup_fd_(::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC)) {

        LUWU_ASSERT(epoll_fd_ != -1);
        LUWU_ASSERT(wakeup_fd_ != -1);

        // 统一事件源，将 wakeup_fd 也加入 epoll 进行管理
        addEvent(wakeup_fd_, ReactorEvent::READ, [&]() {
            uint64_t one = 1;
            ::read(wakeup_fd_, &one, sizeof one);
        });

        channelResize(32);
        // 启动调度器
        start();
    }
    // endregion

    void Reactor::channelResize(size_t size) {
        channels_.resize(size);
        for (int i = 0; i < channels_.size(); ++i) {
            if (!channels_[i]) {
                channels_[i] = new Channel(i);
            }
        }
    }

    Reactor::~Reactor() {
        // 关闭调度器，主线程调度协程开始执行，如果有的话
        stop();
        ::close(epoll_fd_);
        ::close(wakeup_fd_);
        for (auto &channel: channels_) {
            delete channel;
        }
    }

    bool Reactor::addEvent(int fd, ReactorEvent::Event event, const std::function<void()> &cb) {
        // 取出 fd 对应的 channel，如果没有则扩容
        Channel *channel;
        RWMutex::ReadLock lock(mutex_);
        if (channels_.size() > fd) {
            channel = channels_[fd];
            lock.unlock();
        } else {
            lock.unlock();
            RWMutex::WriteLock lock1(mutex_);
            channelResize(fd * 2);
            channel = channels_[fd];
        }

        // 不可以重复注册事件，如果要修改事件对应的回调，则应该先删除再添加
        Mutex::Lock lock1(channel->mutex_);
        LUWU_ASSERT(!(channel->event_ & event));

        // 使用系统调用修改底层 epoll
        epoll_event ev{};
        memset(&ev, 0, sizeof ev);
        ev.events = channel->event_ | event | EPOLLET;
        ev.data.fd = fd;
        ev.data.ptr = channel;
        int op = channel->event_ ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
        int rt = epoll_ctl(epoll_fd_, op, fd, &ev);
        if (rt) {
            LUWU_LOG_ERROR(LUWU_LOG_ROOT()) << "epoll_ctl(" << epoll_fd_ << ", "
                                            << op << ", " << fd << ", " << ev.events << "):"
                                            << rt << "(" << errno << ")(" << strerror(errno) << ")";
            return false;
        }

        // Reactor 模型对应的部分也要修改
        channel->event_ = static_cast<ReactorEvent::Event>(channel->event_ | event);
        Channel::EventCallback &event_callback = channel->getEventCallback(event);
        LUWU_ASSERT(!event_callback.scheduler_ && !event_callback.fiber_ && !event_callback.func_);

        event_callback.scheduler_ = Scheduler::GetThis();
        if (cb) {
            event_callback.func_ = cb;
        } else {
            // 回调函数为空，说明是一个回调函数中途 yield，需要将自己添加到 epoll 中，等待再次执行，所以把当前协程当作回调
            // TODO
            event_callback.fiber_ = Fiber::GetThis()->shared_from_this();
            LUWU_ASSERT(event_callback.fiber_->getState() == Fiber::RUNNING);
        }
        return true;
    }

    bool Reactor::delEvent(int fd, ReactorEvent::Event event) {
        // 取出 fd 对应的 channel，如果没有则出错
        RWMutex::ReadLock lock(mutex_);
        if (channels_.size() <= fd) {
            return false;
        }
        Channel *channel = channels_[fd];
        lock.unlock();

        // 不可以删除没有注册的事件
        Mutex::Lock lock1(channel->mutex_);
        if (!(channel->event_ & event)) {
            return false;
        }

        epoll_event ev{};
        memset(&ev, 0, sizeof ev);
        ev.events = (channel->event_ & ~event) | EPOLLET;
        ev.data.fd = fd;
        ev.data.ptr = channel;
        int op = (channel->event_ & ~event) ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
        int rt = epoll_ctl(epoll_fd_, op, fd, &ev);
        if (rt) {
            LUWU_LOG_ERROR(LUWU_LOG_ROOT()) << "epoll_ctl(" << epoll_fd_ << ", "
                                            << op << ", " << fd << ", " << ev.events << "):"
                                            << rt << "(" << errno << ")(" << strerror(errno) << ")";
            return false;
        }

        channel->event_ = static_cast<ReactorEvent::Event>(channel->event_ & ~event);
        Channel::EventCallback &event_callback = channel->getEventCallback(event);
        Channel::resetEventCallback(event_callback);
        return true;
    }

    Reactor *Reactor::GetThis() {
        return dynamic_cast<Reactor *>(Scheduler::GetThis());
    }

    bool Reactor::stopping() {
        uint64_t timeout = getNextTime();
        // 定时器和调度器都没有任务时才可以停止
        return timeout == ~0ull && Scheduler::stopping();
    }

    void Reactor::idle() {
        static const uint32_t MAX_EVENTS = 256;
        static const uint64_t MAX_TIMEOUT = 3000;
        std::vector<epoll_event> events(MAX_EVENTS);

        while (!stopping()) {
            // 根据定时器确定超时时间
            uint64_t next_timeout = getNextTime();
            if (next_timeout != ~0ull) {
                next_timeout = std::min(next_timeout, MAX_TIMEOUT);
            } else {
                next_timeout = MAX_TIMEOUT;
            }

            // 阻塞等待
            int event_num = epoll_wait(epoll_fd_, &*events.begin(), MAX_EVENTS,
                                       static_cast<int>(next_timeout));

            // TODO 处理信号
            // 退出 epoll_wait 说明有定时器超时或者有事件发生

            // 处理超时的定时器
            std::vector<std::function<void()>> callbacks;
            listExpiredCallback(callbacks);
            for (const auto &callback: callbacks) {
                addTask(callback);
            }

            // 处理到来的事件
            for (int i = 0; i < event_num; ++i) {
                epoll_event &event = events[i];
                auto *channel = static_cast<Channel *>(event.data.ptr);
                Mutex::Lock lock(channel->mutex_);

                // TODO 多种事件的处理
                // 现阶段只处理读写事件
                if (event.events & EPOLLIN) {
                    channel->triggerEvent(ReactorEvent::READ);
                }
                if (event.events & EPOLLOUT) {
                    channel->triggerEvent(ReactorEvent::WRITE);
                }
            }  // end for

            Fiber::GetThis()->yield();
        } // end while
    }

    void Reactor::tickle() {
        uint64_t one = 1;
        ::write(wakeup_fd_, &one, sizeof one);
    }

    void Reactor::onClockInsertAtFront() {
        tickle();
    }
}
