//
// Created by liucxi on 2022/11/4.
//

#include "utils/asserts.h"

using namespace luwu;

int main() {
    LUWU_ASSERT(1 == 2);
    LUWU_ASSERT2(false, "test_asserts");
}