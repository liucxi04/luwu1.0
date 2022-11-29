//
// Created by liucxi on 2022/11/28.
//

#include "http/ws_server.h"

using namespace luwu;

void test_http_server() {
    http::WSServer::ptr server(new http::WSServer);
    Address::ptr addr = IPv4Address::Create("0.0.0.0", 12345);
    server->bind(addr);

    auto dispatch = server->getDispatch();
    dispatch->addExactWSServlet("/luwu", [](http::HttpRequest::ptr header,
                                             http::WSFrameMessage::ptr msg,
                                             http::WSConnection::ptr conn) -> int {

        conn->sendMessage(msg);
        return 0;
    });

    server->start();
}

int main() {
    Reactor r("reactor");
    r.addTask(test_http_server);
    return 0;
}