//
// Created by liucxi on 2022/11/26.
//

#ifndef LUWU_WS_SERVER_H
#define LUWU_WS_SERVER_H

#include "tcp_server.h"
#include "servlet.h"

namespace luwu {
    namespace http {
        /**
         * @brief WS 服务器，继承 TCP 服务器
         */
        class WSServer : public TCPServer {
        public:
            using ptr = std::shared_ptr<WSServer>;

            /**
            * @brief 构造函数
            * @param name ws 服务器名称，也是 tcp 服务器名称
            * @param acceptor acceptor 反应堆
            * @param worker worker 反应堆
            */
            explicit WSServer(const std::string& name = "luwu/1.0.0",
                              Reactor *acceptor = Reactor::GetThis(),
                              Reactor *worker = Reactor::GetThis());

            /**
            * @brief 获取 ws 服务器的 servlet 分发器
            * @return servlet 分发器
            */
            ServletDispatch::ptr getDispatch() { return dispatch_; }

        protected:
            /**
            * @brief ws 服务器的 handle client
            * @param client 需要处理的 socket 客户端连接
            */
            void handleClient(const Socket::ptr &client) override;

        private:
            /// servlet 分发器
            ServletDispatch::ptr dispatch_;
        };
    }
}

#endif //LUWU_WS_SERVER_H
