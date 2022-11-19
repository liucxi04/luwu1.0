//
// Created by liucxi on 2022/11/16.
//

#include "hook.h"

#include <dlfcn.h>
#include <fcntl.h>
#include <cstdarg>
#include "fiber.h"
#include "reactor.h"
#include "file_descriptor.h"

// 带参数的宏定义
#define HOOK_FUN(XX) \
    XX(sleep)        \
    XX(socket)       \
    XX(connect)      \
    XX(accept)       \
    XX(close)        \
    XX(read)         \
    XX(readv)        \
    XX(recv)         \
    XX(recvfrom)     \
    XX(recvmsg)      \
    XX(write)        \
    XX(writev)       \
    XX(send)         \
    XX(sendto)       \
    XX(sendmsg)      \
    XX(fcntl)        \

namespace luwu {
    // 线程局部变量，标识该线程是否被 hook
    static thread_local bool t_is_hooked = false;

    bool isHooked() {
        return t_is_hooked;
    }

    void setHooked(bool flag) {
        t_is_hooked = flag;
    }

    struct HookInit {
        // ## 用于字符串连接， # 表示是一个字符串
        // 以 sleep 为例，展开为 sleep_f = (sleep_fun)dlsym(RTLD_NEXT, "sleep");
        // 表示使用 dlsym() 系统调用找到名为 "sleep" 的系统调用，将其赋值给 sleep_f，sleep_f 在 extern "C" 内初始化
        HookInit() {
#define XX(name) name ## _f = (name ## _fun)dlsym(RTLD_NEXT, #name);
            HOOK_FUN(XX)
#undef XX
        }
    };

    // static 变量会在 main 函数之前被初始化，在 s_hook_init 被构造时会将上述的原始系统调用的地址保存在同名的 name_f 函数指针中。
    static HookInit s_hook_init;

    // 指示定时器是否被取消，用于在回调函数中传递信息
    struct ClockInfo {
        int canceled = 0;
    };

    /**
     * @brief io 类型的系统调用的统一处理模板类
     * @tparam OriginFunc 原始系统调用
     * @tparam Args 系统调用的参数
     * @param fd socket 文件描述符
     * @param func 原始系统调用
     * @param event fd 上发生的事件
     * @param so_timeout 超时类型，如 SO_RCVTIMEO
     * @param args 系统调用的参数
     * @return 读写字节数
     */
    template<typename OriginFunc, typename ... Args>
    static ssize_t do_io(int fd, OriginFunc func, uint32_t event, int so_timeout, Args &&... args) {
        if (!isHooked()) {
            return func(fd, std::forward<Args>(args)...);
        }

        auto ctx = FdMgr::GetInstance().get(fd);
        if (!ctx || !ctx->isInit() || ctx->isClose()) {
            errno = EBADF;
            return -1;
        }
        if (!ctx->isSocket() || ctx->isUserNonblock()) {
            return func(fd, std::forward<Args>(args)...);
        }

        // 执行到这里是 -- 用户没有设置非阻塞的 socket 文件描述符
        // 获得 socket 文件描述符的超时事件，没有设置结果为 0
        timeval tv{};
        socklen_t len = sizeof tv;
        getsockopt(fd, SOL_SOCKET, so_timeout, &tv, &len);
        uint64_t timeout = tv.tv_sec * 1000 + tv.tv_usec / 1000;

        ssize_t n;
        while (true) {
            do {
                // 这里就是非阻塞 socket fd 了，可以立即返回
                n = func(fd, std::forward<Args>(args)...);
            } while (n == -1 && errno == EINTR);            // 立即返回了，但是是中断信号导致的，忽略

            // 立即返回了，但是没有新连接到来或者没有数据可读写
            if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                auto r = Reactor::GetThis();
                std::shared_ptr<ClockInfo> shared_info(new ClockInfo);
                std::weak_ptr<ClockInfo> weak_info(shared_info);        // 指向 shared_info 但不增加引用计数

                Clock::ptr clock;
                // 如果设置了超时时间
                if (timeout != 0) {
                    clock = r->addCondClock(timeout, [r, weak_info, fd, event](){
                        auto t = weak_info.lock();
                        if (t) {
                            t->canceled = ETIMEDOUT;        // 设置超时标志
                        }
                        // 删除前触发一次，使添加该定时器的协程可以 resume
                        r->delEvent(fd, static_cast<ReactorEvent::Event>(event), true);
                    }, weak_info);
                }

                bool rt = r->addEvent(fd, static_cast<ReactorEvent::Event>(event));
                if (rt) {
                    Fiber::GetThis()->yield();
                    // resume 有两种可能：定时器超时，注册的事件到来
                    // 1. 用户没有设置非阻塞，有超时时间，所以在超时之后返回错误
                    if (shared_info->canceled) {
                        errno = shared_info->canceled;
                        return -1;
                    }
                    // 2. 没有超时，是由于事件发生返回的，所以取消定时器，再次接受数据
                    if (clock) {
                        clock->cancel();
                    }
                } else {
                    // 添加事件出错，再次接受数据
                    if (clock) {
                        clock->cancel();
                    }
                }
            } else {
                break;
            }
        }
        return n;
    }
}


// 接口的 hook 实现，要放在 extern "C" 中，以防止 C++ 编译器对符号名称添加修饰。
extern "C" {

// 指向原始系统调用的函数指针初始化
// 以 sleep 为例，展开为 sleep_fun sleep_f = nullptr;
#define XX(name) name ## _fun name ## _f = nullptr;
        HOOK_FUN(XX)
#undef XX

    unsigned int sleep(unsigned int seconds) {
        if (!luwu::isHooked()) {
            return sleep_f(seconds);
        }

        auto fiber = luwu::Fiber::GetThis();
        auto r = luwu::Reactor::GetThis();
        r->addClock(seconds * 1000, [fiber, r](){
            r->addTask(fiber);
        });
        fiber->yield();
        return 0;
    }

    int socket(int domain, int type, int protocol) {
        if (!luwu::isHooked()) {
            return socket_f(domain, type, protocol);
        }

        int fd = socket_f(domain, type, protocol);
        if (fd >= 0) {
            luwu::FdMgr::GetInstance().get(fd, true);
        }
        return fd;
    }

    int connect(int sockfd, const struct sockaddr *addr, socklen_t addlen) {
        if (!luwu::isHooked()) {
            return connect_f(sockfd, addr, addlen);
        }

        auto ctx = luwu::FdMgr::GetInstance().get(sockfd);
        if (!ctx || !ctx->isInit() || ctx->isClose()) {
            errno = EBADF;
            return -1;
        }
        if (!ctx->isSocket() || ctx->isUserNonblock()) {
            return connect_f(sockfd, addr, addlen);
        }

        // 执行到这里是 -- 用户没有设置非阻塞的 socket 文件描述符
        // 获得 socket 文件描述符的超时事件，没有设置结果为 0
        timeval tv{};
        socklen_t len = sizeof tv;
        getsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv, &len);
        uint64_t timeout = tv.tv_sec * 1000 + tv.tv_usec / 1000;

        int n = connect_f(sockfd, addr, addlen);
        if (n == 0) {                                   // 连接成功
            return 0;
        } else if (n == -1 && errno != EINPROGRESS) {   // 连接失败
            // connect 被信号中断之后不能再次 connect，需要返回错误，这也是 connect 函数不能归类到 do_io 的原因
            return -1;
        }

        // 立即返回了，但是没有连接成功
        // n == -1 && errno == EINPROGRESS，表示连接还在进行中
        auto r = luwu::Reactor::GetThis();
        std::shared_ptr<luwu::ClockInfo> shared_info(new luwu::ClockInfo);
        std::weak_ptr<luwu::ClockInfo> weak_info(shared_info);              // 指向 shared_info 但不增加引用计数

        luwu::Clock::ptr clock;
        if (timeout != 0) {
            clock = r->addCondClock(timeout, [r, weak_info, sockfd](){
                auto t = weak_info.lock();
                if (t) {
                    t->canceled = ETIMEDOUT;
                }
                // 删除前触发一次，使添加该定时器的协程可以 resume
                r->delEvent(sockfd, luwu::ReactorEvent::WRITE, true);
            }, weak_info);
        }


        bool rt = r->addEvent(sockfd, luwu::ReactorEvent::WRITE);
        if (rt) {
            luwu::Fiber::GetThis()->yield();
            // resume 有两种可能：定时器超时，写事件到来
            // 1. 超时之后返回错误
            if (shared_info->canceled) {
                errno = shared_info->canceled;
                return -1;
            }
            // 2. 没有超时，是由于事件发生返回的，所以取消定时器
            if (clock) {
                clock->cancel();
            }
        } else {
            // 添加写事件出错
            if (clock) {
                clock->cancel();
            }
        }

        // 执行到这里是 -- connect 连接成功或者添加写事件失败
        int error = 0;
        socklen_t err_len = sizeof error;
        if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &err_len) == -1) {
            return -1;
        }
        if (error == 0) {
            return 0;
        } else {
            errno = error;
            return -1;
        }
    }

    int accept(int sockfd, struct sockaddr *addr, socklen_t *addlen) {
        ssize_t rt = luwu::do_io(sockfd, accept_f,
                                luwu::ReactorEvent::READ, SO_RCVTIMEO, addr, addlen);
        int fd = static_cast<int>(rt);
        if (fd >= 0) {
            luwu::FdMgr::GetInstance().get(fd, true);
        }
        return fd;
    }

    int close(int fd) {
        if (!luwu::isHooked()) {
            return close_f(fd);
        }

        auto ctx = luwu::FdMgr::GetInstance().get(fd);
        if (ctx) {
            auto r = luwu::Reactor::GetThis();
            r->delEvent(fd, luwu::ReactorEvent::READ);
            r->delEvent(fd, luwu::ReactorEvent::WRITE);
        }
        luwu::FdMgr::GetInstance().del(fd);
        return close_f(fd);
    }

    // region # read and write 系列函数
    ssize_t read(int fd, void *buf, size_t count) {
        return luwu::do_io(fd, read_f, luwu::ReactorEvent::READ, SO_RCVTIMEO, buf, count);
    }

    ssize_t readv(int fd, const struct iovec *iov, int iovcnt) {
        return luwu::do_io(fd, readv_f, luwu::ReactorEvent::READ, SO_RCVTIMEO, iov, iovcnt);
    }

    ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
        return luwu::do_io(sockfd, recv_f, luwu::ReactorEvent::READ, SO_RCVTIMEO, buf, len, flags);
    }

    ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,
                          struct sockaddr *src_addr, socklen_t *addrlen) {
        return luwu::do_io(sockfd, recvfrom_f, luwu::ReactorEvent::READ, SO_RCVTIMEO,
                           buf, len, flags, src_addr, addrlen);
    }

    ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags) {
        return luwu::do_io(sockfd, recvmsg_f, luwu::ReactorEvent::READ, SO_RCVTIMEO, msg, flags);
    }

    ssize_t write(int fd, const void *buf, size_t count) {
        return luwu::do_io(fd, write_f, luwu::ReactorEvent::WRITE, SO_SNDTIMEO, buf, count);
    }

    ssize_t writev(int fd, const struct iovec *iov, int iovcnt) {
        return luwu::do_io(fd, writev_f, luwu::ReactorEvent::WRITE, SO_SNDTIMEO, iov, iovcnt);
    }

    ssize_t send(int sockfd, const void *buf, size_t len, int flags) {
        return luwu::do_io(sockfd, send_f, luwu::ReactorEvent::WRITE, SO_SNDTIMEO, buf, len, flags);
    }

    ssize_t sendto(int socket, const void *msg, size_t len, int flags,
                    const struct sockaddr *to, socklen_t tolen) {
        return luwu::do_io(socket, sendto_f, luwu::ReactorEvent::WRITE, SO_SNDTIMEO,
                           msg, len, flags, to, tolen);
    }

    ssize_t sendmsg(int socket, const struct msghdr *msg, int flags) {
        return luwu::do_io(socket, sendmsg_f, luwu::ReactorEvent::WRITE, SO_SNDTIMEO, msg, flags);
    }
    // endregion

    // hook fcntl 的目的是使文件描述符的阻塞状态与用户所设置的一致
    int fcntl(int fd, int cmd, ...) {
        va_list va;
        va_start(va, cmd);
        switch (cmd) {
            case F_SETFL: {
                int arg = va_arg(va, int);
                va_end(va);

                auto ctx = luwu::FdMgr::GetInstance().get(fd);
                if (!ctx || !ctx->init() || ctx->isClose() || !ctx->isSocket()) {
                    return fcntl_f(fd, cmd, arg);
                }

                ctx->setUserNonblock(arg & O_NONBLOCK);
                if (ctx->isSysNonblock()) {
                    arg |= O_NONBLOCK;
                } else {
                    arg &= ~O_NONBLOCK;
                }
                return fcntl_f(fd, cmd, arg);
            }
            case F_GETFL: {
                va_end(va);
                int flags = fcntl_f(fd, cmd);

                auto ctx = luwu::FdMgr::GetInstance().get(fd);
                if (!ctx || !ctx->init() || ctx->isClose() || !ctx->isSocket()) {
                    return flags;
                }

                if (ctx->isUserNonblock()) {
                    return flags | O_NONBLOCK;
                } else {
                    return flags & ~O_NONBLOCK;
                }
            }
            case F_DUPFD:
            case F_DUPFD_CLOEXEC:
            case F_SETFD:
            case F_SETOWN:
            case F_SETSIG:
            case F_SETLEASE:
            case F_NOTIFY:
            case F_SETPIPE_SZ: {
                int arg = va_arg(va, int);
                va_end(va);
                return fcntl_f(fd, cmd, arg);
            }
            case F_GETFD:
            case F_GETOWN:
            case F_GETSIG:
            case F_GETLEASE:
            case F_GETPIPE_SZ: {
                va_end(va);
                return fcntl_f(fd, cmd);
            }
            case F_SETLK:
            case F_SETLKW:
            case F_GETLK: {
                struct flock *arg = va_arg(va, struct flock*);
                va_end(va);
                return fcntl_f(fd, cmd, arg);
            }
            case F_GETOWN_EX:
            case F_SETOWN_EX: {
                struct f_owner_exlock *arg = va_arg(va, struct f_owner_exlock*);
                va_end(va);
                return fcntl_f(fd, cmd, arg);
            }
            default:
                va_end(va);
                return fcntl_f(fd, cmd);
        }
    }
}
