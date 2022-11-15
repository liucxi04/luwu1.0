//
// Created by liucxi on 2022/11/14.
//

#include "reactor.h"

using namespace luwu;

void test_io() {

}

int main() {
    Reactor r("test_reactor");
    r.addTask(test_io);
    return 0;
}