//
// Created by liucxi on 2022/11/14.
//

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include "reactor.h"

using namespace luwu;

int sockfd;
void watch_io_read();

void do_io_write() {
    std::cout << "write callback" << std::endl;
    int error = 0;
    socklen_t len = sizeof error;
    getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len);
    if (error) {
        std::cout << "connect fail" << std::endl;
        return;
    }
    std::cout << "connect success" << std::endl;

    const char data[] = "GET / HTTP/1.0\r\n\r\n";
    send(sockfd, data, sizeof(data), 0);
}

void do_io_read() {
    std::cout << "read callback" << std::endl;
    char buffer[4096] = {0};
    ssize_t rt = read(sockfd, buffer, sizeof buffer);
    if (rt > 0) {
        buffer[rt] = '\0';
        std::cout << "read buffer = " << buffer << std::endl;
    } else if (rt == 0) {
        std::cout << "peer close" << std::endl;
        close(sockfd);
        return;
    } else {
        std::cout << "read fail" << std::endl;
        close(sockfd);
        return;
    }
    // read之后重新添加读事件回调，这里不能直接调用addEvent，因为在当前位置fd的读事件上下文还是有效的，直接调用addEvent相当于重复添加相同事件
    Reactor::GetThis()->addTask(watch_io_read);
}

void test_io() {
    sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);

    sockaddr_in addr{};
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "36.152.44.96", &addr.sin_addr.s_addr);

    int rt = connect(sockfd, (const sockaddr *) &addr, sizeof addr);
    if (rt != 0) {
        if (errno == EINPROGRESS) {
            Reactor::GetThis()->addEvent(sockfd, luwu::ReactorEvent::WRITE, do_io_write);
            Reactor::GetThis()->addEvent(sockfd, luwu::ReactorEvent::READ, do_io_read);
        } else {
            std::cout << "connect error, errno = " << strerror(errno) << std::endl;
        }
    } else {
        std::cout << "connect return immediately" << std::endl;
    }
}

void watch_io_read() {
    std::cout << "watch_io_read" << std::endl;
    Reactor::GetThis()->addEvent(sockfd, luwu::ReactorEvent::READ, do_io_read);
}
// 这里需要将 setHooked(true); 注释掉，否则运行的是 hook 之后的系统调用，connect 会立刻返回
int main() {
    Reactor r("test_reactor");
    r.addTask(test_io);
    return 0;
}