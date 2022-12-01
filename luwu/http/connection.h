//
// Created by liucxi on 2022/11/28.
//

#ifndef LUWU_CONNECTION_H
#define LUWU_CONNECTION_H

#include "../socket.h"
#include "../byte_array.h"

namespace luwu {
    namespace http {
        /**
         * @brief HTTP 连接
         */
        class Connection {
        public:
            using ptr = std::shared_ptr<Connection>;

            /**
             * @brief 构造函数
             * @param socket socket 连接
             */
            explicit Connection(Socket::ptr socket) : socket_(std::move(socket)) { }

            /**
             * @brief 析构函数
             */
            virtual ~Connection() = default;

            /**
            * @brief 从 socket 读取数据到缓冲区
            * @param buffer 缓冲区
            * @param length 缓冲区大小
            * @return 读取字节数
            */
            virtual size_t read(void *buffer, size_t length);

            /**
             * @brief 从 socket 读取数据到字节数组
             * @param byte_array 字节数组
             * @param length 字节数组大小
             * @return 读取字节数
             */
            virtual size_t read(ByteArray::ptr byte_array, size_t length);

            /**
             * @brief 从 socket 读取指定长度的数据到缓冲区
             * @param buffer 缓冲区
             * @param length 缓冲区大小
             * @return 读取字节数
             */
            virtual size_t readFixSize(void *buffer, size_t length);

            /**
             * @brief 从 socket 读取指定长度的数据到字节数组
             * @param byte_array 字节数组
             * @param length 字节数组大小
             * @return 读取字节数
             */
            virtual size_t readFixSize(ByteArray::ptr byte_array, size_t length);

            /**
             * @brief 将缓冲区数据写入到 socket
             * @param buffer 缓冲区
             * @param length 缓冲区大小
             * @return 发送字节数
             */
            virtual size_t write(const void *buffer, size_t length);

            /**
             * @brief 将字节数组数据写入到 socket
             * @param byte_array 字节数组
             * @param length 字节数组大小
             * @return 发送字节数
             */
            virtual size_t write(ByteArray::ptr byte_array, size_t length);

            /**
             * @brief 将缓冲区指定长度的数据写入到 socket
             * @param buffer 缓冲区
             * @param length 缓冲区大小
             * @return 发送字节数
             */
            virtual size_t writeFixSize(const void *buffer, size_t length);

            /**
             * @brief 将字节数组指定长度的数据写入到 socket
             * @param byte_array 字节数组
             * @param length 字节数组大小
             * @return 发送字节数
             */
            virtual size_t writeFixSize(ByteArray::ptr byte_array, size_t length);

            /**
             * @brief 获取 http 连接所持有的 socket 对象
             * @return 所持有的 socket 对象
             */
            Socket::ptr getSocket() { return socket_; }

            bool isConnected() const { return socket_ && socket_->isConnected(); }

        private:
            /// 所持有的 socket 对象
            Socket::ptr socket_;
        };
    }
}


#endif //LUWU_CONNECTION_H
