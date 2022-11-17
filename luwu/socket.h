//
// Created by liucxi on 2022/11/15.
//

#ifndef LUWU_SOCKET_H
#define LUWU_SOCKET_H

#include <memory>
#include "address.h"
#include "utils/noncopyable.h"

namespace luwu {
    class Socket : public std::enable_shared_from_this<Socket>, NonCopyable {
    public:
        using ptr = std::shared_ptr<Socket>;

        static Socket::ptr CreateTCP();

        static Socket::ptr CreateTCP(const Address::ptr &addr);

        static Socket::ptr CreateUDP();

        static Socket::ptr CreateUDP(const Address::ptr &addr);

        Socket(int family, int type, int protocol = 0);

        ~Socket();

        uint64_t  getSendTimeout() const;

        void setSendTimeout(uint64_t timeout);

        uint64_t  getRecvTimeout() const;

        void setRecvTimeout(uint64_t timeout);

        template<typename T>
        bool getOption(int level, int option, T &value) const {
            socklen_t len = sizeof value;
            return getOption(level, option, &value, &len);
        }

        template<typename T>
        bool setOption(int level, int option, const T &value) {
            return setOption(level, option, &value, sizeof value);
        }

        bool enableRead();

        bool enableWrite();

        bool cancelRead();

        bool cancelWrite();

        // region # Socket system call
        bool bind(const Address::ptr& addr);

        bool listen(int backlog = SOMAXCONN);

        Socket::ptr accept();

        bool connect(const Address::ptr& addr);

        bool reconnect();

        bool close();
        // endregion

        // region # Send and Recv
        size_t send(const void *buffer, size_t length, int flags = 0);

        size_t send(const iovec *buffer, size_t length, int flags = 0);

        size_t sendTo(const void *buffer, size_t length, const Address::ptr &to, int flags = 0);

        size_t sendTo(const iovec *buffer, size_t length, const Address::ptr &to, int flags = 0);

        size_t recv(void *buffer, size_t length, int flags = 0);

        size_t recv(iovec *buffer, size_t length, int flags = 0);

        size_t recvFrom(void *buffer, size_t length, const Address::ptr &from, int flags = 0);

        size_t recvFrom(iovec *buffer, size_t length, const Address::ptr &from, int flags = 0);
        // endregion

        // region # Getter
        int getFd() const { return fd_; }

        int getFamily() const { return family_; }

        int getType() const { return type_; }

        int getProtocol() const { return protocol_; }

        bool isConnected() const { return connected_; }

        Address::ptr getLocalAddress() { return local_address; }

        Address::ptr getPeerAddress() {return peer_address; }

        int getError() const;
        // endregion

        std::ostream &dump(std::ostream &os) const;

        std::string toString() const;

    private:
        bool getOption(int level, int option, void *result, socklen_t *len) const;

        bool setOption(int level, int option, const void *result, socklen_t len);

        void setNonblock();

        Socket(int fd, int family, int type, int protocol);

        void setLocalAddress();

        void setPeerAddress();

    private:
        int fd_;
        int family_;
        int type_;
        int protocol_;
        bool connected_;

        Address::ptr local_address;
        Address::ptr peer_address;
    };
}

#endif //LUWU_SOCKET_H
