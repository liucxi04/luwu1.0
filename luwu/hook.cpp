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
    static thread_local bool t_is_hooked = false;

    bool isHooked() {
        return t_is_hooked;
    }

    void setHooked(bool flag) {
        t_is_hooked = flag;
    }

    struct HookInit {
        HookInit() {
#define XX(name) name ## _f = (name ## _fun)dlsym(RTLD_NEXT, #name);
            HOOK_FUN(XX)
#undef XX
        }
    };

    static HookInit s_hook_init;

    struct ClockInfo {
        int canceled = 0;
    };

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

        timeval tv{};
        socklen_t len = sizeof tv;
        getsockopt(fd, SOL_SOCKET, so_timeout, &tv, &len);
        uint64_t timeout = tv.tv_sec * 1000 + tv.tv_usec / 1000;

        auto r = Reactor::GetThis();
        std::shared_ptr<ClockInfo> shared_info(new ClockInfo);
        std::weak_ptr<ClockInfo> weak_info(shared_info);

        ssize_t n;
        while (true) {
            do {
                n = func(fd, std::forward<Args>(args)...);
            } while (n == -1 && errno == EINTR);

            if (n == -1 && errno == EAGAIN) {
                Clock::ptr clock;
                if (timeout != 0) {
                    clock = r->addCondClock(timeout, [r, weak_info, fd, event](){
                        auto t = weak_info.lock();
                        if (t) {
                            t->canceled = ETIMEDOUT;
                        }
                        r->delEvent(fd, static_cast<ReactorEvent::Event>(event));
                    }, weak_info);
                }

                int rt = r->addEvent(fd, static_cast<ReactorEvent::Event>(event));
                if (rt) {
                    Fiber::GetThis()->yield();
                    if (clock) {
                        clock->cancel();
                    }
                    if (shared_info->canceled) {
                        errno = shared_info->canceled;
                        return -1;
                    }
                } else {
                    if (clock) {
                        clock->cancel();
                    }
                    // 添加事件出错
                }
            } else {
                break;
            }
        }
        return n;
    }
}

extern "C" {

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

        int n = connect_f(sockfd, addr, addlen);
        if (n == 0) {
            return 0;           /// ??? 什么情况会发生
        } else if (n != -1 || errno != EINPROGRESS) {
            return n;
        }

        auto r = luwu::Reactor::GetThis();
        std::shared_ptr<luwu::ClockInfo> shared_info(new luwu::ClockInfo);
        std::weak_ptr<luwu::ClockInfo> weak_info(shared_info);

        static uint64_t connect_timeout = 5000;
        /// 理解
        auto clock = r->addCondClock(connect_timeout, [r, weak_info, sockfd](){
            auto t = weak_info.lock();
            if (t) {
                t->canceled = ETIMEDOUT;
            }
            r->delEvent(sockfd, luwu::ReactorEvent::WRITE);
        }, weak_info);

        bool rt = r->addEvent(sockfd, luwu::ReactorEvent::WRITE);
        if (rt) {
            luwu::Fiber::GetThis()->yield();
            if (clock) {
                clock->cancel();
            }
            if (shared_info->canceled) {
                errno = shared_info->canceled;
                return -1;
            }
        } else {
            if (clock) {
                clock->cancel();
            }
            // 添加写事件出错
        }

        int error = 0;
        socklen_t len = sizeof error;
        if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len) == -1) {
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
            close_f(fd);
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

    ssize_t read(int fd, void *buf, size_t count) {
        return luwu::do_io(fd, read_f, luwu::ReactorEvent::READ, SO_RCVTIMEO, buf, count);
    }

    ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
        return luwu::do_io(sockfd, recv_f, luwu::ReactorEvent::READ, SO_RCVTIMEO, buf, len, flags);
    }

    ssize_t readv(int fd, const struct iovec *iov, int iovcnt) {
        return luwu::do_io(fd, readv_f, luwu::ReactorEvent::READ, SO_RCVTIMEO, iov, iovcnt);
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

    ssize_t send(int sockfd, const void *buf, size_t len, int flags) {
        return luwu::do_io(sockfd, send_f, luwu::ReactorEvent::WRITE, SO_SNDTIMEO, buf, len, flags);
    }

    ssize_t writev(int fd, const struct iovec *iov, int iovcnt) {
        return luwu::do_io(fd, writev_f, luwu::ReactorEvent::WRITE, SO_SNDTIMEO, iov, iovcnt);
    }

    ssize_t sendto(int socket, const void *msg, size_t len, int flags,
                    const struct sockaddr *to, socklen_t tolen) {
        return luwu::do_io(socket, sendto_f, luwu::ReactorEvent::WRITE, SO_SNDTIMEO,
                           msg, len, flags, to, tolen);
    }

    ssize_t sendmsg(int socket, const struct msghdr *msg, int flags) {
        return luwu::do_io(socket, sendmsg_f, luwu::ReactorEvent::WRITE, SO_SNDTIMEO, msg, flags);
    }

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
