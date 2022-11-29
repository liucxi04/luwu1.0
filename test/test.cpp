//
// Created by liucxi on 2022/11/16.
//
#include <iostream>
#include <cstring>
#include <sstream>
#include "utils/endian.h"

#pragma pack(1)
struct WSFrameHead {
    bool fin_:1;
    bool rsv1_:1;
    bool rsv2_:1;
    bool rsv3_:1;
    uint32_t opcode_:4;
    bool mask_:1;
    uint32_t payload_:7;

    std::string toString() const {
        std::stringstream ss;
        ss << "[WSFrameHead fin = " << fin_
           << " rsv1 = " << rsv1_ << " rsv2 = " << rsv2_ << " rsv3 = " << rsv3_
           << " opcode = " << luwu::onBigEndian(opcode_)
           << " mask = " << mask_ << " payload = " << luwu::onBigEndian(payload_) << "]";
        return ss.str();
    }
};
#pragma pack()

int main() {
    WSFrameHead head{};
    uint8_t data[2] = {0x81, 0x81};

    memcpy(&head, &data, sizeof head);
    std::cout << head.toString() << std::endl;
    uint32_t opcode = data[0] & 0x0f;
    std::cout << opcode << std::endl;
    return 0;
}
