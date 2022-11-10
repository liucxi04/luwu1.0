//
// Created by liucxi on 2022/11/10.
//

#include "clock.h"
#include <iostream>

using namespace luwu;

void test_clock1() {
    std::cout << "test_clock1" << std::endl;
}
class IOM : public ClockManager {
public:
    ~IOM() override = default;

private:
    void onClockInsertAtFront() override {

    }
};

int main() {
    IOM iom;
    iom.addClock(3, test_clock1, true);
    return 0;
}
