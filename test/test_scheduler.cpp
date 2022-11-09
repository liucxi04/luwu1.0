//
// Created by liucxi on 2022/11/8.
//

#include "scheduler.h"
#include "utils/util.h"
#include <iostream>

using namespace luwu;

void test_scheduler1() {
    std::cout << getThreadId() << ", " << Fiber::GetFiberId() << " test_scheduler1 begin" << std::endl;

    Scheduler::GetThis()->addTask(Fiber::GetThis()->shared_from_this());            // 退出前必须将自己再加入协程调度器
    std::cout << "before test_scheduler1 yield" << std::endl;
    Fiber::GetThis()->yield();
    std::cout << "after test_scheduler1 yield" << std::endl;

    std::cout << getThreadId() << ", " << Fiber::GetFiberId() << " test_scheduler1 end" << std::endl;
}

void test_scheduler2() {
    std::cout << getThreadId() << ", " << Fiber::GetFiberId() << " test_scheduler2 begin" << std::endl;

    // sleep(3);

    std::cout << getThreadId() << ", " << Fiber::GetFiberId() << " test_scheduler2 end" << std::endl;
}

void test_scheduler3() {
    std::cout << getThreadId() << ", " << Fiber::GetFiberId() << " test_scheduler3 begin" << std::endl;

    std::cout << getThreadId() << ", " << Fiber::GetFiberId() << " test_scheduler3 end" << std::endl;
}

void test_scheduler4() {
    static int count = 0;
    std::cout << getThreadId() << ", " << Fiber::GetFiberId() << " test_scheduler1 begin, i = " << count << std::endl;
    std::cout << getThreadId() << ", " << Fiber::GetFiberId() << " test_scheduler4 end, i = " << count << std::endl;
    ++count;
}

void test_scheduler5() {
    std::cout << getThreadId() << ", " << Fiber::GetFiberId() << " test_scheduler5 begin" << std::endl;
    for (int i = 0; i < 3; ++i) {
        Scheduler::GetThis()->addTask(test_scheduler4, getThreadId());
    }
    std::cout << getThreadId() << ", " << Fiber::GetFiberId() << " test_scheduler5 end" << std::endl;
}

int main() {
    //std::cout << getThreadId() << ", " << Fiber::GetFiberId() << " main begin" << std::endl;
    // Scheduler scheduler("scheduler");
    // Scheduler scheduler("scheduler", 1, false);
    Scheduler scheduler("scheduler", 1, true);

    scheduler.addTask(test_scheduler1);
    scheduler.addTask(test_scheduler2);

    scheduler.addTask(std::make_shared<Fiber>(test_scheduler3));

    scheduler.start();

    scheduler.addTask(test_scheduler5);
    scheduler.stop();

    //std::cout << getThreadId() << ", " << Fiber::GetFiberId() << " main end" << std::endl;
    return 0;
}
