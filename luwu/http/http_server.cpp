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
        }

        void HttpServer::handleClient(const Socket::ptr &client) {
            LUWU_LOG_DEBUG(LUWU_LOG_ROOT()) << client->toString();
            HttpConnection::ptr conn(new HttpConnection(client));
            do {
                auto req = conn->recvRequest();
                if (!req) {
                    LUWU_LOG_ERROR(LUWU_LOG_ROOT()) << "recv http request failed, errno = " << errno
                                                    << " errstr = " << strerror(errno)
                                                    << "client : " << client->toString();
                    break;
                }
                bool close = req->isClose() || !keepalive_;
                HttpResponse::ptr rsp(new HttpResponse(req->getVersion(), close));
                dispatch_->handle(req, rsp, conn);
                conn->sendResponse(rsp);
                if (close) {
                    break;
                }
            } while (true);
        }
    }
}