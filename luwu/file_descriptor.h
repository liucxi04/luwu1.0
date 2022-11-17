//
// Created by liucxi on 2022/11/16.
//

#ifndef LUWU_FILE_DESCRIPTOR_H
#define LUWU_FILE_DESCRIPTOR_H

#include <memory>
#include <vector>
#include "utils/mutex.h"
#include "utils/singleton.h"

namespace luwu {
    class FdContext {
    public:
        using ptr = std::shared_ptr<FdContext>;

        explicit FdContext(int fd);

        bool init();

        // region # Getter and Setter
        bool isInit() const {
            return is_init_;
        }

        bool isSocket() const {
            return is_socket_;
        }

        bool isSysNonblock() const {
            return is_sys_nonblock_;
        }

        bool isUserNonblock() const {
            return is_user_nonblock;
        }

        bool isClose() const {
            return is_close_;
        }

        void setSysNonblock(bool isSysNonblock) {
            is_sys_nonblock_ = isSysNonblock;
        }

        void setUserNonblock(bool isUserNonblock) {
            is_user_nonblock = isUserNonblock;
        }
        // endregion

    private:
        int fd_;
        bool is_init_:1;
        bool is_socket_:1;
        bool is_sys_nonblock_:1;
        bool is_user_nonblock:1;
        bool is_close_:1;
    };

    class FdManager {
    public:
        FdManager();

        FdContext::ptr get(int fd, bool auto_create = false);

        void del(int fd);

    private:
        RWMutex mutex_;
        std::vector<FdContext::ptr> fds_;
    };

    using FdMgr = Singleton<FdManager>;
}


#endif //LUWU_FILE_DESCRIPTOR_H
