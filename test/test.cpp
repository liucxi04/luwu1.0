//
// Created by liucxi on 2022/11/16.
//
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
int main() {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    std::cout << fd << std::endl;
    close(fd);
    std::cout << fd << std::endl;
    return 0;
}
