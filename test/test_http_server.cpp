//
// Created by liucxi on 2022/11/24.
//

#include "http/http_server.h"

using namespace luwu;

void test_http_server() {
    http::HttpServer::ptr server(new http::HttpServer);
    Address::ptr addr = IPv4Address::Create("0.0.0.0", 12345);
    server->bind(addr);

    auto dispatch = server->getDispatch();
    dispatch->addExactServlet("/luwu/xx", [](http::HttpRequest::ptr req,
                                            http::HttpResponse::ptr rsp,
                                            http::HttpConnection::ptr conn) -> int {

        rsp->setBody(req->toString());
        return 0;
    });

    dispatch->addFuzzyServlet("/luwu/*", [](http::HttpRequest::ptr req,
                                           http::HttpResponse::ptr rsp,
                                           http::HttpConnection::ptr conn) -> int {
        rsp->setBody(req->toString());
        return 0;
    });

    server->start();
}

int main() {
    Reactor r("reactor");
    r.addTask(test_http_server);
    return 0;
}