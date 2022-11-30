//
// Created by liucxi on 2022/11/21.
//

#include "tcp_server.h"

#include <cstring>
#include "logger.h"
#include "utils/asserts.h"

namespace luwu {
    TCPServer::TCPServer(std::string name, Reactor *acceptor, Reactor *worker)
        : name_(std::move(name)), acceptor_(acceptor), worker_(worker), stop_(false) {
        LUWU_LOG_INFO(LUWU_LOG_ROOT()) << "create a new tcp server, name = " << getName();
    }

    TCPServer::~TCPServer() {
        if (!stop_) {
            stop();
        }
        LUWU_LOG_INFO(LUWU_LOG_ROOT()) << "tcp server stop, name = " << getName();
    }

    bool TCPServer::bind(const Address::ptr &address) {
        sock_ = Socket::CreateTCP(address->getFamily());
        if (!sock_->bind(address)) {
            LUWU_LOG_ERROR(LUWU_LOG_ROOT()) << "bind filed errno=" << errno
                                            << " errstr=" << strerror(errno)
                                            << "addr=[" << address->toString() << "";
            return false;
        }
        if (!sock_->listen()) {
            LUWU_LOG_ERROR(LUWU_LOG_ROOT()) << "listen filed errno=" << errno
                                            << " errstr=" << strerror(errno)
                                            << "addr=[" << address->toString() << "";
            return false;
        }
        return true;
    }

    void TCPServer::start() {
        LUWU_ASSERT(!stop_);
        acceptor_->addTask(std::bind(&TCPServer::handleAccept, shared_from_this()));
    }

    void TCPServer::stop() {
        stop_ = true;
        acceptor_->addTask([this](){
            sock_->cancelRead();
            sock_->close();
        });
    }

    void TCPServer::handleAccept() {
        while (!stop_) {
            Socket::ptr client = sock_->accept();
            if (client) {
                client->setRecvTimeout(s_recv_timeout);
                client->setSendTimeout(s_send_timeout);
                worker_->addTask(std::bind(&TCPServer::handleClient, shared_from_this(), client));
            } else {
                LUWU_LOG_ERROR(LUWU_LOG_ROOT()) << "accept errno = " << errno
                                                 << " errstr = " << strerror(errno);
            }
        }
    }

    void TCPServer::handleClient(const Socket::ptr &client) {
        LUWU_LOG_INFO(LUWU_LOG_ROOT()) << "handleClient" << client->toString();
    }
}