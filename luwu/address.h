//
// Created by liucxi on 2022/11/15.
//

#ifndef LUWU_ADDRESS_H
#define LUWU_ADDRESS_H

#include <memory>
#include <sys/socket.h>
#include <arpa/inet.h>

namespace luwu {

    class Address {
    public:
        using ptr = std::shared_ptr<Address>;

        static Address::ptr Create(const sockaddr *addr);

        Address() = default;

        virtual ~Address() = default;

        virtual const sockaddr *getAddr() const = 0;

        virtual sockaddr *getAddr() = 0;

        virtual socklen_t getAddrLen() const = 0;

        int getFamily() const;

        virtual std::ostream &dump(std::ostream &os) const = 0;

        std::string toString() const;
    };

    class IPAddress : public Address {
    public:
        using ptr = std::shared_ptr<IPAddress>;

        virtual uint16_t getPort() const = 0;
    };

    class IPv4Address : public IPAddress {
    public:
        using ptr = std::shared_ptr<IPv4Address>;

        static IPv4Address::ptr Create(const std::string& address, uint16_t port = 0);

        explicit IPv4Address(sockaddr_in address);

        explicit IPv4Address(uint32_t address = INADDR_ANY, uint16_t port = 0);

        ~IPv4Address() override = default;

        const sockaddr *getAddr() const override;

        sockaddr *getAddr() override;

        socklen_t getAddrLen() const override;

        std::ostream &dump(std::ostream &os) const override;

        uint16_t getPort() const override;

    private:
        sockaddr_in addr_{};
    };

    // TODO 系列网络操作
}

#endif //LUWU_ADDRESS_H
