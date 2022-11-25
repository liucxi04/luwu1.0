//
// Created by liucxi on 2022/11/24.
//

#ifndef LUWU_HTTP_SERVER_H
#define LUWU_HTTP_SERVER_H

#include "http_servlet.h"
#include "../tcp_server.h"

namespace luwu {
    namespace http {
        /**
         * @brief HTTP 服务器，继承 TCP 服务器
         */
        class HttpServer : public TCPServer {
        public:
            using ptr = std::shared_ptr<HttpServer>;

            /**
             * @brief 构造函数
             * @param name http 服务器名称，也是 tcp 服务器名称
             * @param acceptor acceptor 反应堆
             * @param worker worker 反应堆
             * @param keepalive 是否保持长连接，默认为 true
             */
            explicit HttpServer(const std::string& name = "luwu/1.0.0",
                                Reactor *acceptor = Reactor::GetThis(), Reactor *worker = Reactor::GetThis(),
                                bool keepalive = false);

            /**
             * @brief 获取 http 服务器的 servlet 分发器
             * @return servlet 分发器
             */
            ServletDispatch::ptr getDispatch() { return dispatch_; }

        protected:
            /**
             * @brief http 服务器的 handle client
             * @param client 需要处理的 socket 客户端连接
             */
            void handleClient(const Socket::ptr &client) override;

        private:
            /// 是否保持长连接，http 请求与本设置只要有一个是长连接即为长连接
            bool keepalive_;
            /// servlet 分发器
            ServletDispatch::ptr dispatch_;
        };
    }
}
#endif //LUWU_HTTP_SERVER_H
