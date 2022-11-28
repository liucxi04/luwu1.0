//
// Created by liucxi on 2022/11/24.
//

#ifndef LUWU_HTTP_CONNECTION_H
#define LUWU_HTTP_CONNECTION_H

#include <utility>
#include "message.h"
#include "connection.h"

namespace luwu {
    namespace http {
        /**
         * @brief HTTP 连接
         */
        class HttpConnection : public Connection {
        public:
            using ptr = std::shared_ptr<HttpConnection>;

            /**
             * @brief 构造函数
             * @param socket http 连接所持有的 socket
             */
            explicit HttpConnection(Socket::ptr socket) : Connection(std::move(socket)) { }

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
        };
    }
}
#endif //LUWU_HTTP_CONNECTION_H
