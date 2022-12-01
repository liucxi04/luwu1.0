//
// Created by liucxi on 2022/11/26.
//

#include <iostream>
#include "../logger.h"
#include "ws_server.h"

namespace luwu {
    namespace http {
        WSServer::WSServer(const std::string &name, Reactor *acceptor, Reactor *worker)
            : TCPServer(name, acceptor, worker)
            , dispatch_(new ServletDispatch(name + "_dispatch")){
            LUWU_LOG_INFO(LUWU_LOG_ROOT()) << "create a new websocket server, name = " << getName();
        }

        void WSServer::handleClient(const Socket::ptr &client) {
            WSConnection::ptr conn(new WSConnection(client));
            LUWU_LOG_INFO(LUWU_LOG_ROOT()) << "websocket handle client, client address = "
                                           << client->getPeerAddress()->toString();

            auto rsp = conn->handleShake();
            if (!rsp) {
                LUWU_LOG_ERROR(LUWU_LOG_ROOT()) << "websocket handle shake return nullptr! client address = "
                                                << client->getPeerAddress()->toString();
                return;
            }

            auto servlet = dispatch_->getMatchedServlet(rsp->getPath());
            if (!servlet) {
                LUWU_LOG_ERROR(LUWU_LOG_ROOT()) << "websocket get servlet return nullptr! request path = "
                                                << rsp->getPath();
                return;
            }

            while (true) {
                auto msg = conn->recvMessage();
                if (!msg) {
                    LUWU_LOG_INFO(LUWU_LOG_ROOT()) << "websocket receive message return nullptr! handle client over"
                                                   << " client address = " << client->getPeerAddress()->toString();
                    break;
                }
                int rt = servlet->handle(rsp, msg, conn);
                if (rt == -1) {
                    LUWU_LOG_ERROR(LUWU_LOG_ROOT()) << "websocket handle request return -1! request = "
                                                    << rsp->toString();
                    break;
                }
            }
        }
    }
}
