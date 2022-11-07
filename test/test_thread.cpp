//
// Created by liucxi on 2022/11/6.
//

#include <unistd.h>
#include <iostream>
#include "thread.h"
#include "utils/mutex.h"
#include "utils/util.h"

using namespace luwu;

int count = 0;
Mutex s_mutex;

void thread_fun1() {
    std::cout << "in thread1, name = " << Thread::GetThis()->getName() << std::endl;
    for (int i = 0; i < 10; ++i) {
        sleep(1);
        std::cout << "in thread1:" << i << std::endl;
    }
}

void thread_fun2() {
    std::cout << "in thread2, name = " << Thread::GetThis()->getName() << std::endl;
    for (int i = 0; i < 5; ++i) {
        sleep(2);
        std::cout << "in thread2:" << i << std::endl;
    }
}

void func3() {
    std::cout << "in thread3: name = " << Thread::GetThis()->getName() << ", " << getThreadName() << std::endl;
    std::cout << "in thread3: id = " << Thread::GetThis()->getId() << ", " << getThreadId() << std::endl;

    for(int i = 0; i < 10000; i++) {
        Mutex::Lock lock(s_mutex);
        ++count;
    }
}

int main() {
//    Thread t1("thread1", thread_fun1);
//    Thread t2("aaa", thread_fun2);
//    t1.join();
//    t2.join();
    std::vector<Thread::ptr> threads;
    for(int i = 0; i < 3; i++) {
        Thread::ptr thr(new Thread("thread_" + std::to_string(i), func3));
        threads.push_back(thr);
    }

    for(int i = 0; i < 3; i++) {
        threads[i]->join();
    }

    std::cout << "count = " << count;
    return 0;
}