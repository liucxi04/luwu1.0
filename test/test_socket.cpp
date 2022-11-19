//
// Created by liucxi on 2022/11/17.
//

#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>
#include <cstring>
#include "reactor.h"
#include "socket.h"

using namespace luwu;

void test_sock() {
    IPAddress::ptr addr = IPv4Address::Create("36.152.44.96", 80);
    if (!addr) {
        std::cout << "create addr failed!" << std::endl;
    } else {
        std::cout << "create addr success, addr = " << addr->toString() << std::endl;
    }

    Socket::ptr sock = Socket::CreateTCP();
    std::cout << "create socket success, socket = " << sock->toString() << std::endl;

    int rt = sock->connect(addr);
    if (!rt) {
        std::cout << "connect addr failed!" << std::endl;
    } else {
        std::cout << "local addr = " << sock->getLocalAddress()->toString()
                  << ", peer addr = " << sock->getPeerAddress()->toString() << std::endl;
    }

    const char data[] = "GET / HTTP/1.0\r\n\r\n";
    sock->send(data, sizeof data);

    char buffer[4096] = {0};
    size_t rt2 = sock->recv(buffer, sizeof buffer);
    buffer[rt2] = '\0';
    std::cout << "recv buffer = " << buffer << std::endl;
}

int main(int argc, char **argv) {
    Reactor r("socket");
    r.addTask(test_sock);
    return 0;
}