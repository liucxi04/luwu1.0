//
// Created by liucxi on 2022/11/21.
//

#include "tcp_server.h"
#include <iostream>
#include <utility>

using namespace luwu;

class MyServer : public TCPServer {
public:
    explicit MyServer(std::string name) : TCPServer(std::move(name)) {}

protected:
    void handleClient(const Socket::ptr &client) override {
        std::cout << "new client" << client->toString() << std::endl;
        std::string buf;
        buf.resize(4096);
        size_t len = client->recv(&buf[0], buf.size());
        buf.resize(len);
        std::cout << "client recv = " << buf << std::endl;
        client->close();
    }
};

void run() {
    TCPServer::ptr server(new MyServer("server"));
    Address::ptr addr = IPv4Address::Create("127.0.0.1", 12345);
    if (!addr) {
        std::cout << "create addr failed!" << std::endl;
    }
    if (!server->bind(addr)) {
        std::cout << "bind addr failed!" << std::endl;
    }
    server->start();
//    server->stop();
}
int main() {
    Reactor r("reactor");
    r.addTask(run);
    return 0;
}