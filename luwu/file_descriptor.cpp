//
// Created by liucxi on 2022/11/16.
//

#include "file_descriptor.h"
#include <sys/stat.h>
#include <fcntl.h>

namespace luwu {
    FdContext::FdContext(int fd)
        : fd_(fd), is_init_(false), is_socket_(false)
        , is_sys_nonblock_(false), is_user_nonblock(false), is_close_(false) {
        init();
    }

    bool FdContext::init() {
        if (is_init_) {
            return true;
        }

        struct stat fd_stat{};
        if (fstat(fd_, &fd_stat) == -1) {
            is_init_ = false;
            is_socket_ = false;
        } else {
            is_init_ = false;
            is_socket_ = S_ISSOCK(fd_stat.st_mode);
        }

        is_user_nonblock = false;
        is_close_ = false;

        if (is_socket_) {
            int flags = fcntl(fd_, F_GETFL, 0);
            fcntl(fd_, F_SETFL, flags | O_NONBLOCK);
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
        if (fd == -1) {
            return nullptr;
        }

        RWMutex::ReadLock lock(mutex_);
        if (fd >= fds_.size()) {
            if (!auto_create) {
                return nullptr;
            }
        } else {
            if (fds_[fd] || !auto_create) {
                return fds_[fd];
            }
        }
        lock.unlock();

        RWMutex::WriteLock lock1(mutex_);
        if (fd >= fds_.size()) {
            fds_.resize(fd * 2);
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