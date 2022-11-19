//
// Created by liucxi on 2022/11/16.
//

#include "file_descriptor.h"
#include <sys/stat.h>
#include <fcntl.h>
#include "hook.h"

namespace luwu {
    FdContext::FdContext(int fd)
        : fd_(fd), is_init_(false), is_socket_(false)
        , is_sys_nonblock_(false), is_user_nonblock(false), is_close_(false) {
        init();
    }

    bool FdContext::init() {
        if (isInit()) {
            return true;
        }

        // 通过系统调用获得文件描述符的状态
        struct stat fd_stat{};
        if (fstat(fd_, &fd_stat) == -1) {        // fstat 调用出错
            is_init_ = false;
            is_socket_ = false;
        } else {
            is_init_ = true;
            is_socket_ = S_ISSOCK(fd_stat.st_mode);
        }

        // TODO 用户使用类似 socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0) 时怎么办？
        is_user_nonblock = false;
        is_close_ = false;

        if (is_socket_) {                                               // socket 描述符系统统一设置为非阻塞
            // 这两个必须直接指定使用原始系统调用，使用 fcntl 会被 hook，造成死锁
            int flags = fcntl_f(fd_, F_GETFL, 0);
            fcntl_f(fd_, F_SETFL, flags | O_NONBLOCK);
            is_sys_nonblock_ = true;
        } else {
            is_sys_nonblock_ = false;
        }
        return is_init_;
    }

    FdManager::FdManager() {
        fds_.resize(64);
    }

    FdContext::ptr FdManager::get(int fd, bool auto_create) {
        if (fd <= 0) {
            return nullptr;
        }

        // 先处理不需要创建的情况
        // 1. 超界，不自动创建
        // 2. 不超界，有值，无需创建
        // 3. 不超界，无值，不自动创建
        RWMutex::ReadLock lock(mutex_);
        if (fd >= fds_.size()) {
            if (!auto_create) {
                return nullptr;                 // 1、
            }
        } else {
            if (fds_[fd] || !auto_create) {
                return fds_[fd];                // 2、3
            }
        }
        lock.unlock();

        // 然后处理需要创建的情况
        // 4. 超界，需要创建
        // 5. 不超界，无值，需要创建
        RWMutex::WriteLock lock1(mutex_);
        if (fd >= fds_.size()) {
            fds_.resize(fd * 2);        // 4、
        }
        FdContext::ptr ctx(new FdContext(fd));
        fds_[fd] = ctx;
        return ctx;
    }

    void FdManager::del(int fd) {
        RWMutex::WriteLock lock(mutex_);
        if (fd >= fds_.size()) {
            return;
        }
        fds_[fd].reset();
    }
}