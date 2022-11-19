//
// Created by liucxi on 2022/11/15.
//

#include "socket.h"
#include <unistd.h>
#include <netinet/tcp.h>        // for TCP_NODELAY
#include <cstring>
#include "utils/asserts.h"
#include "file_descriptor.h"
#include "reactor.h"

namespace luwu {
    Socket::ptr Socket::CreateTCP(int family) {
        Socket::ptr sock(new Socket(family, SOCK_STREAM, 0));
        return sock;
    }

    Socket::ptr Socket::CreateUDP(int family) {
        Socket::ptr sock(new Socket(family, SOCK_DGRAM, 0));
        return sock;
    }

    // region Socket::Socket()
    Socket::Socket(int family, int type, int protocol)
        : fd_(::socket(family, type, protocol))
        , family_(family), type_(type), protocol_(protocol), connected_(false) {
        LUWU_ASSERT(fd_ != -1);
        setReuseAndNodelay();
    }
    // endregion

    Socket::~Socket() {
        close();
    }

    uint64_t Socket::getSendTimeout() const {
        timeval tv{};
        getOption(SOL_SOCKET, SO_RCVTIMEO, tv);
        uint64_t timeout = tv.tv_sec * 1000 + tv.tv_usec / 1000;
        return timeout == 0 ? -1 : timeout;
    }

    void Socket::setSendTimeout(uint64_t timeout) {
        timeval tv{static_cast<int>(timeout / 1000), static_cast<int>(timeout % 1000 * 1000)};
        setOption(SOL_SOCKET, SO_RCVTIMEO, &tv);
    }

    uint64_t Socket::getRecvTimeout() const {
        timeval tv{};
        getOption(SOL_SOCKET, SO_SNDTIMEO, tv);
        uint64_t timeout = tv.tv_sec * 1000 + tv.tv_usec / 1000;
        return timeout == 0 ? -1 : timeout;
    }

    void Socket::setRecvTimeout(uint64_t timeout) {
        timeval tv{static_cast<int>(timeout / 1000), static_cast<int>(timeout % 1000 * 1000)};
        setOption(SOL_SOCKET, SO_SNDTIMEO, &tv);
    }

    bool Socket::bind(const Address::ptr& addr) {
        LUWU_ASSERT(fd_ != -1);
        if (addr->getFamily() != family_) {
            return false;
        }
        if (::bind(fd_, addr->getAddr(), addr->getAddrLen())) {
            close();
            return false;
        }
        getLocalAddress();
        return true;
    }

    bool Socket::listen(int backlog) {
        LUWU_ASSERT(fd_ != -1);
        if (::listen(fd_, backlog)) {
            close();
            return false;
        }
        return true;
    }

    Socket::ptr Socket::accept() {
        LUWU_ASSERT(fd_ != -1);
        int conn_fd = ::accept(fd_, nullptr, nullptr);
        if (conn_fd == -1) {
            return nullptr;
        }
        Socket::ptr sock(new Socket(conn_fd, family_, type_, protocol_));
        sock->connected_ = true;
        sock->setLocalAddress();
        sock->setPeerAddress();
        return sock;
    }

    bool Socket::connect(const Address::ptr& addr) {
        LUWU_ASSERT(fd_ != -1);
        if (addr->getFamily() != family_) {
            return false;
        }
        if (::connect(fd_, addr->getAddr(), addr->getAddrLen())) {
            close();
            return false;
        }
        connected_ = true;
        setLocalAddress();
        peer_address = addr;
        return true;
    }

    bool Socket::close() {
        if (fd_ == -1) {
            return false;
        }
        connected_ = false;
        ::close(fd_);
        fd_ = -1;
        return true;
    }

    size_t Socket::send(const void *buffer, size_t length, int flags) {
        if (isConnected()) {
            return ::send(fd_, buffer, length, flags);
        }
        return -1;
    }

    size_t Socket::send(const iovec *buffer, size_t length, int flags) {
        if (isConnected()) {
            msghdr msg{};
            memset(&msg, 0, sizeof msg);
            msg.msg_iov = const_cast<iovec *>(buffer);
            msg.msg_iovlen = length;
            return ::sendmsg(fd_, &msg, flags);
        }
        return -1;
    }

    size_t Socket::sendTo(const void *buffer, size_t length, const Address::ptr &to, int flags) {
        if (isConnected()) {
            return sendto(fd_, buffer, length, flags, to->getAddr(), to->getAddrLen());
        }
        return -1;
    }

    size_t Socket::sendTo(const iovec *buffer, size_t length, const Address::ptr &to, int flags) {
        if (isConnected()) {
            msghdr msg{};
            memset(&msg, 0, sizeof msg);
            msg.msg_iov = const_cast<iovec *>(buffer);
            msg.msg_iovlen = length;
            msg.msg_name = to->getAddr();
            msg.msg_namelen = to->getAddrLen();
            return ::sendmsg(fd_, &msg, flags);
        }
        return -1;
    }

    size_t Socket::recv(void *buffer, size_t length, int flags) {
        if (isConnected()) {
            return ::recv(fd_, buffer, length, flags);
        }
        return -1;
    }

    size_t Socket::recv(iovec *buffer, size_t length, int flags) {
        if (isConnected()) {
            msghdr msg{};
            memset(&msg, 0, sizeof msg);
            msg.msg_iov = static_cast<iovec *>(buffer);
            msg.msg_iovlen = length;
            return ::recvmsg(fd_, &msg, flags);
        }
        return -1;
    }

    size_t Socket::recvFrom(void *buffer, size_t length, const Address::ptr &from, int flags) {
        if (isConnected()) {
            socklen_t len = from->getAddrLen();
            return ::recvfrom(fd_, buffer, length, flags, from->getAddr(), &len);
        }
        return -1;
    }

    size_t Socket::recvFrom(iovec *buffer, size_t length, const Address::ptr &from, int flags) {
        if (isConnected()) {
            msghdr msg{};
            memset(&msg, 0, sizeof msg);
            msg.msg_iov = buffer;
            msg.msg_iovlen = length;
            msg.msg_name = from->getAddr();
            msg.msg_namelen = from->getAddrLen();
            return ::recvmsg(fd_, &msg, flags);
        }
        return -1;
    }

    int Socket::getError() const {
        int error = 0;
        if (!getOption(SOL_SOCKET, SO_ERROR, error)) {
            return -1;
        }
        return error;
    }

    bool Socket::cancelRead() {
        return Reactor::GetThis()->delEvent(fd_, ReactorEvent::READ, true);
    }

    bool Socket::cancelWrite() {
        return Reactor::GetThis()->delEvent(fd_, ReactorEvent::WRITE, true);
    }

    std::ostream &Socket::dump(std::ostream &os) const {
        os << "[Socket sock = " << fd_
           << " is_connected = " << connected_
           << " family = " << family_
           << " type = " << type_
           << " protocol = " << protocol_;
        if (local_address) {
            os << " local_address = " << local_address->toString();
        }
        if (peer_address) {
            os << " remote_address = " << peer_address->toString();
        }
        os << "]";
        return os;
    }

    std::string Socket::toString() const {
        std::stringstream ss;
        dump(ss);
        return ss.str();
    }

    bool Socket::getOption(int level, int option, void *result, socklen_t *len) const {
        int rt = getsockopt(fd_, level, option, result, len);
        if (rt) {
            return false;
        }
        return true;
    }

    bool Socket::setOption(int level, int option, const void *result, socklen_t len) {
        int rt = setsockopt(fd_, level, option, result, len);
        if (rt) {
            return false;
        }
        return true;
    }

    void Socket::setReuseAndNodelay() {
        int val = 1;
        setOption(SOL_SOCKET, SO_REUSEADDR, val);
        if (type_ == SOCK_STREAM) {
            setOption(IPPROTO_TCP, TCP_NODELAY, val);
        }
    }

    // region # Socket::Socket(fd)
    Socket::Socket(int fd, int family, int type, int protocol)
            : fd_(fd), family_(family), type_(type), protocol_(protocol), connected_(false){
        LUWU_ASSERT(fd_ != -1);
        setReuseAndNodelay();
    }
    // endregion

    void Socket::setLocalAddress() {
        if (local_address) {
            return;
        }

        Address::ptr addr;
        switch (family_) {
            case AF_INET:
                addr.reset(new IPv4Address);
                break;
            default:
                addr.reset();
                break;
        }

        socklen_t addrlen = addr->getAddrLen();
        if (getsockname(fd_, addr->getAddr(), &addrlen) == 0) {
            local_address = addr;
        }
    }

    void Socket::setPeerAddress() {
        if (peer_address) {
            return;
        }

        Address::ptr addr;
        switch (family_) {
            case AF_INET:
                addr.reset(new IPv4Address);
                break;
            default:
                addr.reset();
                break;
        }

        socklen_t addrlen = addr->getAddrLen();
        if (getpeername(fd_, addr->getAddr(), &addrlen) == 0) {
            local_address = addr;
        }
    }
}