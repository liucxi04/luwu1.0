//
// Created by liucxi on 2022/11/10.
//

#include "clock.h"
#include <iostream>
#include "reactor.h"

using namespace luwu;

static int timeout = 1000;
static Clock::ptr s_clock;

void timer_callback() {
    std::cout << "timer callback, timeout = " << timeout << std::endl;
    timeout += 1000;
    if(timeout < 5000) {
        s_clock->reset(timeout, true);
    } else {
        s_clock->cancel();
    }
}

int main() {
    Reactor r("reactor");
    // 循环定时器
    s_clock = r.addClock(1000, timer_callback, true);
    // 单次定时器
    r.addClock(500, [](){
        std::cout << "500ms timeout" << std::endl;
    });
    r.addClock(5000, [](){
        std::cout << "5000ms timeout" << std::endl;
    });

    return 0;
}
