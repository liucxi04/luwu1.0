//
// Created by liucxi on 2022/11/16.
//

#ifndef LUWU_HOOK_H
#define LUWU_HOOK_H

#include <sys/socket.h>

namespace luwu {
    /**
     * @brief 获取当前线程的 hook 状态
     * @return 当前线程是否被 hook
     */
    bool isHooked();

    /**
     * @brief 设置当前线程的 hook 状态
     * @param flag 是否 hook
     */
    void setHooked(bool flag);
}

extern "C" {
/**
 * 1. 定义一个函数指针，以 name_fun 命名
 * 2. 定义一个该类型的函数，指向原始系统调用，因为其定义在 hook.cpp，所以使用 extern 关键字指示
 */

/// sleep 系列函数
typedef unsigned int (*sleep_fun)(unsigned int seconds);
extern sleep_fun sleep_f;

/// socket 系列函数
typedef int (*socket_fun)(int domain, int type, int protocol);
extern socket_fun socket_f;

typedef int (*connect_fun)(int sockfd, const struct sockaddr *addr, socklen_t addlen);
extern connect_fun connect_f;

typedef int (*accept_fun)(int sockfd, struct sockaddr *addr, socklen_t *addlen);
extern accept_fun accept_f;

typedef int (*close_fun)(int fd);
extern close_fun close_f;

/// read 系列函数
typedef ssize_t (*read_fun)(int fd, void *buf, size_t count);
extern read_fun read_f;

typedef ssize_t (*readv_fun)(int fd, const struct iovec *iov, int iovcnt);
extern readv_fun readv_f;

typedef ssize_t (*recv_fun)(int sockfd, void *buf, size_t len, int flags);
extern recv_fun recv_f;

typedef ssize_t (*recvfrom_fun)(int sockfd, void *buf, size_t len, int flags,
                                struct sockaddr *src_addr, socklen_t *addrlen);
extern recvfrom_fun recvfrom_f;

typedef ssize_t (*recvmsg_fun)(int sockfd, struct msghdr *msg, int flags);
extern recvmsg_fun recvmsg_f;

/// write 系列函数
typedef ssize_t (*write_fun)(int fd, const void *buf, size_t count);
extern write_fun write_f;

typedef ssize_t (*writev_fun)(int fd, const struct iovec *iov, int iovcnt);
extern writev_fun writev_f;

typedef ssize_t (*send_fun)(int sockfd, const void *buf, size_t len, int flags);
extern send_fun send_f;

typedef ssize_t (*sendto_fun)(int socket, const void *msg, size_t len, int flags,
                              const struct sockaddr *to, socklen_t tolen);
extern sendto_fun sendto_f;

typedef ssize_t (*sendmsg_fun)(int socket, const struct msghdr *msg, int flags);
extern sendmsg_fun sendmsg_f;

/// fcntl
typedef int (*fcntl_fun)(int fd, int cmd, ...);
extern fcntl_fun fcntl_f;

}
#endif //LUWU_HOOK_H
