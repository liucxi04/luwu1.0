//
// Created by liucxi on 2022/11/4.
//

#include <thread>
#include <iostream>
#include "fiber.h"

using namespace luwu;

void run_in_fiber1() {
    std::cout << "run in fiber1 start" << std::endl;
    std::cout << "run in fiber1 yield" << std::endl;
    Fiber::GetThis()->yield();
    std::cout << "run in fiber1 end" << std::endl;
}

void run_in_fiber2() {
    std::cout << "run in fiber2 start" << std::endl;
    std::cout << "run in fiber2 end" << std::endl;
}

// 线程函数
void test_fiber() {
    std::cout << "test_fiber start" << std::endl;

    Fiber::ptr fiber(new Fiber(run_in_fiber1, false));

    std::cout << "do something" << std::endl;
    std::cout << "test_fiber yield" << std::endl;
    fiber->resume();
    std::cout << "test_fiber resume and yield" << std::endl;

    fiber->resume();

    std::cout << "test_fiber switch to run in fiber2" << std::endl;
    fiber->reset(run_in_fiber2);
    fiber->resume();

    std::cout << "test_fiber end" << std::endl;
}
int main() {
    std::thread t(test_fiber);
    t.join();
    return 0;
}