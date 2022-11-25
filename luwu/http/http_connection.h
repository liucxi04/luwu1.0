//
// Created by liucxi on 2022/11/24.
//

#ifndef LUWU_HTTP_CONNECTION_H
#define LUWU_HTTP_CONNECTION_H

#include <utility>
#include "http.h"
#include "socket.h"
#include "byte_array.h"

namespace luwu {
    namespace http {
        /**
         * @brief HTTP 连接
         */
        class HttpConnection {
        public:
            using ptr = std::shared_ptr<HttpConnection>;

            /**
             * @brief 构造函数
             * @param socket http 连接所持有的 socket
             */
            explicit HttpConnection(Socket::ptr  socket) : socket_(std::move(socket)) { }

            /**
             * @brief 接收并解析 http 请求到 HttpRequest 中
             * @return HttpRequest
             */
            HttpRequest::ptr recvRequest();

            /**
             * @brief 发送 http 响应
             * @param rsp http 响应
             * @return 成功发送的字节大小
             */
            size_t sendResponse(const HttpResponse::ptr& rsp);

            /**
             * @brief 从 socket 读取数据到缓冲区
             * @param buffer 缓冲区
             * @param length 缓冲区大小
             * @return 读取字节数
             */
            size_t read(void *buffer, size_t length);

            /**
             * @brief 从 socket 读取数据到字节数组
             * @param byte_array 字节数组
             * @param length 字节数组大小
             * @return 读取字节数
             */
            size_t read(ByteArray::ptr byte_array, size_t length);

            /**
             * @brief 将缓冲区数据写入到 socket
             * @param buffer 缓冲区
             * @param length 缓冲区大小
             * @return 发送字节数
             */
            size_t write(const void *buffer, size_t length);

            /**
             * @brief 将字节数组数据写入到 socket
             * @param byte_array 字节数组
             * @param length 字节数组大小
             * @return 发送字节数
             */
            size_t write(ByteArray::ptr byte_array, size_t length);

            Socket::ptr getSocket() { return socket_; }

            bool isConnected() const { return socket_ && socket_->isConnected(); }

        private:
            /// 所持有的 socket 对象
            Socket::ptr socket_;
        };
    }
}
#endif //LUWU_HTTP_CONNECTION_H
