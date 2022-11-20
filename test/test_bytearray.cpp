//
// Created by liucxi on 2022/11/20.
//

#include "byte_array.h"
#include <random>
#include <vector>
#include <algorithm>
#include <iostream>

using namespace luwu;

void test_num() {
#define XX(type, len, write_fun, read_fun) \
    {                                                 \
        std::vector<type> vec;                        \
        for (int i = 0; i < (len); ++i) {             \
            vec.push_back(rand());                    \
            std::cout << vec[i] << ",";               \
        }                                             \
        std::cout << std::endl;                       \
        ByteArray::ptr ba(new ByteArray);             \
        for (auto &i : vec) {                         \
            ba->write_fun(i);                         \
        }                                             \
        for (int i = 0; i < vec.size(); ++i) {        \
            type v = ba->read_fun();                  \
            std::cout << v << ",";                    \
        }                                             \
        std::cout << std::endl;                       \
    }

    XX(uint8_t , 10, writeInt<uint8_t>, readInt<uint8_t>)
    XX(int8_t , 10, writeInt<int8_t>, readInt<int8_t>)
    XX(uint16_t , 10, writeInt<uint16_t>, readInt<uint16_t>)
    XX(int16_t , 10, writeInt<int16_t>, readInt<int16_t>)
    XX(uint32_t , 10, writeInt<uint32_t>, readInt<uint32_t>)
    XX(int32_t , 10, writeInt<int32_t>, readInt<int32_t>)
    XX(uint64_t , 10, writeInt<uint64_t>, readInt<uint64_t>)
    XX(int64_t , 10, writeInt<int64_t>, readInt<int64_t>)

    XX(double , 10, writeDouble, readDouble)
#undef XX
}

void test_string() {
    std::string s = "qwertyuiopasdfghjklzxcvbnm";
    std::vector<std::string> vec;
    for (int i = 0; i < 10; i++) {
        shuffle(s.begin(), s.end(), std::mt19937(std::random_device()()));
        vec.push_back(s);
    }

    ByteArray::ptr ba(new ByteArray);
    for (auto &i : vec) {
        ba->writeString(i);
    }
    for (auto & i : vec) {
        std::string v = ba->readString();
        std::cout << i << std::endl;
        std::cout << v << std::endl;
    }
}

int main() {
//    test_num();
    test_string();
    return 0;
}