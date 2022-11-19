//
// Created by liucxi on 2022/11/15.
//

#ifndef LUWU_SOCKET_H
#define LUWU_SOCKET_H

#include <memory>
#include "address.h"
#include "utils/noncopyable.h"

namespace luwu {
    /**
     * @brief 套接字封装
     */
    class Socket : public std::enable_shared_from_this<Socket>, NonCopyable {
    public:
        using ptr = std::shared_ptr<Socket>;

        /**
         * @brief 创建一个 TCP 套接字
         * @param family 协议簇，默认为 AF_INET
         * @return 套接字
         */
        static Socket::ptr CreateTCP(int family = AF_INET);

        /**
         * @brief 创建一个 UDP 套接字
         * @param family 协议簇，默认为 AF_INET
         * @return 套接字
         */
        static Socket::ptr CreateUDP(int family = AF_INET);

        /**
         * @brief 构造函数
         * @param family 协议簇
         * @param type 套接字类型
         * @param protocol 传输协议，一般默认 0
         */
        Socket(int family, int type, int protocol = 0);

        /**
         * @brief 析构函数
         */
        ~Socket();

        // region # timeout Getter and Setter
        /**
         * @brief 获取发送超时时间
         * @return 发送超时时间，未设置值为 -1
         */
        uint64_t  getSendTimeout() const;

        /**
         * @brief 设置发送超时时间
         * @param timeout 发送超时时间
         */
        void setSendTimeout(uint64_t timeout);

        /**
         * @brief 获取接收超时时间
         * @return 发送接收超时时间，未设置值为 -1
         */
        uint64_t  getRecvTimeout() const;

        /**
         * @brief 设置接收超时时间
         * @param timeout 接收超时时间
         */
        void setRecvTimeout(uint64_t timeout);
        // endregion

        /**
         * @brief 获取 socket 选项值
         * @tparam T 返回值类型
         * @param level 协议层次
         * @param option 选项名
         * @param value 返回值
         * @return 操作是否成功
         */
        template<typename T>
        bool getOption(int level, int option, T &value) const {
            socklen_t len = sizeof value;
            return getOption(level, option, &value, &len);
        }

        /**
         * @brief 设置 socket 选项值
         * @tparam T 值类型
         * @param level 协议层次
         * @param option 选项名
         * @param value 选项值
         * @return 操作是否成功
         */
        template<typename T>
        bool setOption(int level, int option, const T &value) {
            return setOption(level, option, &value, sizeof value);
        }

        // region # Socket system call
        /**
         * @brief 绑定网络地址
         * @param addr 网络地址
         * @return 操作是否成功
         */
        bool bind(const Address::ptr& addr);

        /**
         * @brief 等待网络连接
         * @param backlog 等待队列最大长度
         * @return 操作是否成功
         */
        bool listen(int backlog = SOMAXCONN);

        /**
         * @brief 接收网络连接
         * @return 新连接的套接字
         */
        Socket::ptr accept();

        /**
         * @brief 发起网络连接
         * @param addr 目标网络地址
         * @return 操作是否成功
         */
        bool connect(const Address::ptr& addr);

        /**
         * @brief 关闭套接字
         * @return 操作是否成功
         */
        bool close();
        // endregion

        // region # Send and Recv
        size_t send(const void *buffer, size_t length, int flags = 0);

        size_t send(const iovec *buffer, size_t length, int flags = 0);

        size_t sendTo(const void *buffer, size_t length, const Address::ptr &to, int flags = 0);

        size_t sendTo(const iovec *buffer, size_t length, const Address::ptr &to, int flags = 0);

        size_t recv(void *buffer, size_t length, int flags = 0);

        size_t recv(iovec *buffer, size_t length, int flags = 0);

        size_t recvFrom(void *buffer, size_t length, const Address::ptr &from, int flags = 0);

        size_t recvFrom(iovec *buffer, size_t length, const Address::ptr &from, int flags = 0);
        // endregion

        // region # Getter
        int getFd() const { return fd_; }

        int getFamily() const { return family_; }

        int getType() const { return type_; }

        int getProtocol() const { return protocol_; }

        bool isConnected() const { return connected_; }

        Address::ptr getLocalAddress() { return local_address; }

        Address::ptr getPeerAddress() {return peer_address; }

        int getError() const;
        // endregion

        /**
         * @brief 取消读事件，取消前触发一次
         * @return 操作是否成功
         */
        bool cancelRead();

        /**
        * @brief 取消写事件，取消前触发一次
        * @return 操作是否成功
        */
        bool cancelWrite();

        /**
         * @brief 格式化到输出流
         * @param os 输出流
         * @return 输出流
         */
        std::ostream &dump(std::ostream &os) const;

        /**
         * @brief 格式化为字符串
         * @return 格式化字符串
         */
        std::string toString() const;

    private:
        /**
         * @brief 获取 socket 选项值实际操作
         * @param level 协议层次
         * @param option 选项名
         * @param result 返回值
         * @param len 返回值长度
         * @return 操作是否成功
         */
        bool getOption(int level, int option, void *result, socklen_t *len) const;

        /**
         * @brief 设置 socket 选项值实际操作
         * @param level 协议层次
         * @param option 选项名
         * @param result 选项值
         * @param len 选项值长度
         * @return 操作是否成功
         */
        bool setOption(int level, int option, const void *result, socklen_t len);

        /**
         * @brief 设置地址重用和 TCP_NODELAY
         */
        void setReuseAndNodelay();

        /**
         * @brief 私有构造函数，根据现有的 fd 构造套接字
         * @param fd socket 文件描述符
         * @param family 协议簇
         * @param type 套接字类型
         * @param protocol 传输协议
         */
        Socket(int fd, int family, int type, int protocol);

        /**
         * @brief 设置本地地址
         */
        void setLocalAddress();

        /**
         * @brief 设置远程地址
         */
        void setPeerAddress();

    private:
        /// 套接字描述符
        int fd_;
        /// 协议簇
        int family_;
        /// 套接字类型
        int type_;
        /// 传输协议
        int protocol_;
        /// 是否连接
        bool connected_;
        /// 本地地址
        Address::ptr local_address;
        /// 远端地址
        Address::ptr peer_address;
    };
}

#endif //LUWU_SOCKET_H
