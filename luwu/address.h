//
// Created by liucxi on 2022/11/15.
//

#ifndef LUWU_ADDRESS_H
#define LUWU_ADDRESS_H

#include <memory>
#include <sys/socket.h>
#include <arpa/inet.h>

namespace luwu {

    /**
     * @details 网络地址基类，抽象类
     */
    class Address {
    public:
        using ptr = std::shared_ptr<Address>;

        /**
         * @brief 通过 sockaddr * 创建对应的 Address
         * @param addr sockaddr 地址
         * @return Address::ptr
         */
        static Address::ptr Create(const sockaddr *addr);

        /**
         * @brief 默认构造函数
         */
        Address() = default;

        /**
         * @brief 虚析构函数
         */
        virtual ~Address() = default;

        /**
         * @brief 获取原始地址，只读
         * @return sockaddr 地址
         */
        virtual const sockaddr *getAddr() const = 0;

        /**
         * @brief 获取原始地址，可写
         * @return sockaddr 地址
         */
        virtual sockaddr *getAddr() = 0;

        /**
         * @brief 获取原始地址的长度
         * @return 地址长度
         */
        virtual socklen_t getAddrLen() const = 0;

        /**
         * @brief 获取地址的协议簇
         * @return 地址的协议簇
         */
        int getFamily() const;

        /**
         * @brief 将数据写入 ostream，配合 toString
         * @param os 输出流
         * @return 输出流
         */
        virtual std::ostream &dump(std::ostream &os) const = 0;

        /**
         * @brief 将地址格式化为字符串
         * @return 字符串
         */
        std::string toString() const;
    };

    /**
     * @brief IP 地址类，抽象类
     */
    class IPAddress : public Address {
    public:
        using ptr = std::shared_ptr<IPAddress>;

        /**
         * @brief 获取端口号
         * @return 端口号
         */
        virtual uint16_t getPort() const = 0;
    };

    /**
     * @brief IPv4 地址
     */
    class IPv4Address : public IPAddress {
    public:
        using ptr = std::shared_ptr<IPv4Address>;

         /**
          * @brief 通过地址和端口号创建对应的 IPv4Address
          * @param address 点分十进制地址字符串
          * @param port 端口号
          * @return IPv4Address
          */
        static IPv4Address::ptr Create(const char *address, uint16_t port = 0);

        /**
         * @brief 构造函数
         * @param address sockaddr_in 地址
         */
        explicit IPv4Address(const sockaddr_in &address);

        /**
         * @brief 构造函数
         * @param address 32为地址
         * @param port 端口号
         */
        explicit IPv4Address(uint32_t address = INADDR_ANY, uint16_t port = 0);

        /**
         * @brief 默认析构函数
         */
        ~IPv4Address() override = default;

        /**
         * @brief 获取原始地址，只读
         * @return sockaddr 地址
         */
        const sockaddr *getAddr() const override;

        /**
         * @brief 获取原始地址，可写
         * @return sockaddr 地址
         */
        sockaddr *getAddr() override;

        /**
         * @brief 获取原始地址的长度
         * @return 地址长度
         */
        socklen_t getAddrLen() const override;

        /**
         * @brief 将数据写入 ostream，配合 toString
         * @param os 输出流
         * @return 输出流
         */
        std::ostream &dump(std::ostream &os) const override;

        /**
         * @brief 获取端口号
         * @return 端口号
         */
        uint16_t getPort() const override;

    private:
        /// IPv4 原始地址
        sockaddr_in addr_{};
    };

    // TODO 系列网络操作
}

#endif //LUWU_ADDRESS_H
