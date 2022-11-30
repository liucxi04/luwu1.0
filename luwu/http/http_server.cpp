//
// Created by liucxi on 2022/11/24.
//

#include <iostream>
#include "http_server.h"
#include "logger.h"

namespace luwu {
    namespace http {
        HttpServer::HttpServer(const std::string& name, Reactor *acceptor, Reactor *worker, bool keepalive)
            : TCPServer(name, acceptor, worker)
            , keepalive_(keepalive), dispatch_(new ServletDispatch(name + "_dispatch")) {
            LUWU_LOG_INFO(LUWU_LOG_ROOT()) << "create a new http server, name = " << getName();
        }

        void HttpServer::handleClient(const Socket::ptr &client) {
            HttpConnection::ptr conn(new HttpConnection(client));
            LUWU_LOG_INFO(LUWU_LOG_ROOT()) << "http handle client, client address = "
                                           << client->getPeerAddress()->toString();

            do {
                auto req = conn->recvRequest();
                if (!req) {
                    LUWU_LOG_ERROR(LUWU_LOG_ROOT()) << "http receive request return nullptr! client address = "
                                                    << "client : " << client->getPeerAddress()->toString();
                    break;
                }
                bool close = req->isClose() || !keepalive_;
                HttpResponse::ptr rsp(new HttpResponse(req->getVersion(), close));
                int rt = dispatch_->handle(req, rsp, conn);
                if (rt == -1) {
                    LUWU_LOG_ERROR(LUWU_LOG_ROOT()) << "http handle client return -1! request path = " << req->getPath();
                    break;
                }
                conn->sendResponse(rsp);
                if (close) {
                    LUWU_LOG_INFO(LUWU_LOG_ROOT()) << "http handle client over, client address = "
                                                   << client->getPeerAddress()->toString();
                    break;
                }
            } while (true);
        }
    }
}