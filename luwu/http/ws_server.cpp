//
// Created by liucxi on 2022/11/26.
//

#include <iostream>
#include "ws_server.h"

namespace luwu {
    namespace http {
        WSServer::WSServer(const std::string &name, Reactor *acceptor, Reactor *worker)
            : TCPServer(name, acceptor, worker)
            , dispatch_(new ServletDispatch(name + "_dispatch")){
        }

        void WSServer::handleClient(const Socket::ptr &client) {
            WSConnection::ptr conn(new WSConnection(client));

            auto rsp = conn->handelShake();
            if (!rsp) {
                return;
            }
            auto servlet = dispatch_->getMatchedServlet(rsp->getPath());
            if (!servlet) {
                return;
            }
            while (true) {
                auto msg = conn->recvMessage();
                if (!msg) {
                    break;
                }
                int rt = servlet->handle(rsp, msg, conn);
                if (rt == -1) {
                    break;
                }
            }
        }
    }
}
