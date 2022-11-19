//
// Created by liucxi on 2022/11/15.
//

#include "address.h"

#include <sstream>
#include <cstring>
#include "utils/asserts.h"
#include "utils/endian.h"

namespace luwu {

    Address::ptr Address::Create(const sockaddr *addr) {
        if (!addr) {
            return nullptr;
        }

        Address::ptr address;
        switch (addr->sa_family) {
            case AF_INET:
                address.reset(new IPv4Address(*(sockaddr_in *)(addr)));
                break;
            default:
                address.reset();
                break;
        }
        return address;
    }

    int Address::getFamily() const {
        return getAddr()->sa_family;
    }

    std::string Address::toString() const {
        std::stringstream ss;
        dump(ss);
        return ss.str();
    }

    IPv4Address::ptr IPv4Address::Create(const char *address, uint16_t port) {
        IPv4Address::ptr addr(new IPv4Address);
        addr->addr_.sin_port = onBigEndian(port);
        int rt = inet_pton(AF_INET, address, &addr->addr_.sin_addr);
        LUWU_ASSERT(rt > 0);
        return addr;
    }

    IPv4Address::IPv4Address(const sockaddr_in &address) : addr_(address) { }

    IPv4Address::IPv4Address(uint32_t address, uint16_t port) {
        memset(&addr_, 0, sizeof addr_);
        addr_.sin_family = AF_INET;
        addr_.sin_addr.s_addr = onBigEndian(address);
        addr_.sin_port = onBigEndian(port);
    }

    const sockaddr *IPv4Address::getAddr() const {
        return (sockaddr *) &addr_;
    }

    sockaddr *IPv4Address::getAddr() {
        return (sockaddr *) &addr_;
    }

    socklen_t IPv4Address::getAddrLen() const {
        return sizeof addr_;
    }

    std::ostream &IPv4Address::dump(std::ostream &os) const {
        uint32_t address = onBigEndian(addr_.sin_addr.s_addr);
        os << ((address >> 24) & 0xff) << "."
           << ((address >> 16) & 0xff) << "."
           << ((address >> 8 ) & 0xff) << "."
           << ((address      ) & 0xff);
        os << ":" << onBigEndian(addr_.sin_port);
        return os;
    }

    uint16_t IPv4Address::getPort() const {
        return addr_.sin_port;
    }
}