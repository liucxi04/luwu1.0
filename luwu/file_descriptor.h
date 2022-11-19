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
    /**
     * @brief 文件描述符上下文
     */
    class FdContext {
    public:
        using ptr = std::shared_ptr<FdContext>;

        /**
         * @brief 构造函数
         * @param fd 文件描述符
         */
        explicit FdContext(int fd);

        /**
         * @brief 初始化文件描述符上下文的所有状态
         * @return 操作是否成功
         */
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

        void setUserNonblock(bool isUserNonblock) {
            is_user_nonblock = isUserNonblock;
        }
        // endregion

    private:
        /// 文件描述符
        int fd_;
        /// 文件描述符上下文状态是否初始化
        bool is_init_:1;
        /// 是否是 socket 文件描述符
        bool is_socket_:1;
        /// hook 模块是否设置了非阻塞
        bool is_sys_nonblock_:1;
        /// 用户是否设置了非阻塞
        bool is_user_nonblock:1;
        /// 文件描述符是否关闭
        bool is_close_:1;           // 其实用不上，因为 close 之后就会从 FdManager 中删除了
    };

    /**
     * @brief 文件描述符上下文管理类
     */
    class FdManager {
    public:
        /**
         * @brief 构造函数
         */
        FdManager();

        /**
         * @brief 获取或创建一个文件描述符上下文
         * @param fd 文件描述符
         * @param auto_create 文件描述符上下文不存在时是否创建
         * @return 对应的文件描述符上下文
         */
        FdContext::ptr get(int fd, bool auto_create = false);

        /**
         * @ 删除一个文件描述符上下文
         * @param fd 文件描述符
         */
        void del(int fd);

    private:
        /// 文件描述符不是属于某一个线程的，所以要加锁
        RWMutex mutex_;
        /// 文件描述符上下文数组
        std::vector<FdContext::ptr> fds_;
    };

    /// 文件描述符上下文管理类的单例
    using FdMgr = Singleton<FdManager>;
}


#endif //LUWU_FILE_DESCRIPTOR_H
