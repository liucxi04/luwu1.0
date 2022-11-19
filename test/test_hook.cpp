//
// Created by liucxi on 2022/11/17.
//

#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>
#include <cstring>
#include "reactor.h"

using namespace luwu;

void test_sleep() {
    std::cout << "test_sleep begin" << std::endl;

    Reactor r("sleep");
    r.addTask([](){
        sleep(3);
        std::cout << "test_sleep 30" << std::endl;
    });
    r.addTask([](){
        sleep(2);
        std::cout << "test_sleep 29" << std::endl;
    });
    std::cout << "test_sleep end" << std::endl;
}

void test_sock() {
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr{};
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "36.152.44.96", &addr.sin_addr.s_addr);

    std::cout << "connect begin" << std::endl;
    int rt = connect(fd, (const sockaddr *) &addr, sizeof addr);
    std::cout << "connect rt = " << rt << ", err = " << strerror(errno) << std::endl;

    if (rt) {
        return;
    }

    const char data[] = "GET / HTTP/1.0\r\n\r\n";
    ssize_t rt1 = send(fd, data, sizeof(data), 0);
    std::cout << "send rt = " << rt1 << ", err = " << strerror(errno) << std::endl;

    if (rt1 <= 0) {
        return;
    }

    char buffer[4096] = {0};
    ssize_t rt2 = recv(fd, buffer, sizeof buffer, 0);
    std::cout << "recv rt = " << rt2 << ", errno = " << errno << std::endl;

    if (rt2 <= 0) {
        return;
    }
    buffer[rt2] = '\0';
    std::cout << "recv buffer = " << buffer << std::endl;
}

int main() {
//     test_sleep();

    Reactor r("socket");
    r.addTask(test_sock);

//    test_sock();
    return 0;
}