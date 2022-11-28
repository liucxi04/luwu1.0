//
// Created by liucxi on 2022/11/28.
//

#include "connection.h"

namespace luwu {
    namespace http {
        size_t Connection::read(void *buffer, size_t length) {
            if (!isConnected()) {
                return -1;
            }
            return socket_->recv(buffer, length);
        }

        size_t Connection::read(ByteArray::ptr byte_array, size_t length) {
            if (!isConnected()) {
                return -1;
            }
            std::string buffer;
            buffer.resize(length);
            size_t len = socket_->recv(&buffer[0], buffer.size());
            buffer.resize(len);
            byte_array->write(buffer.data(), len);
            return len;
        }

        size_t Connection::readFixSize(void *buffer, size_t length) {
            size_t offset = 0;
            while (length > 0) {
                size_t len = read(static_cast<char *>(buffer) + offset, length);
                if (len <= 0) {
                    break;
                }
                offset += len;
                length -= len;
            }
            return offset;
        }

        size_t Connection::readFixSize(ByteArray::ptr byte_array, size_t length) {
            size_t offset = 0;
            while (length > 0) {
                size_t len = read(byte_array, length);
                if (len <= 0) {
                    break;
                }
                offset += len;
                length -= len;
            }
            return offset;
        }

        size_t Connection::write(const void *buffer, size_t length) {
            if (!isConnected()) {
                return -1;
            }
            return socket_->send(buffer, length);
        }

        size_t Connection::write(ByteArray::ptr byte_array, size_t length) {
            if (!isConnected()) {
                return -1;
            }
            std::string buffer;
            buffer.resize(length);
            byte_array->read(&buffer[0], length);
            size_t len = socket_->send(buffer.data(), buffer.size());
            return len;
        }

        size_t Connection::writeFixSize(const void *buffer, size_t length) {
            size_t offset = 0;
            while (length > 0) {
                size_t len = write(static_cast<const char *>(buffer) + offset, length);
                if (len <= 0) {
                    break;
                }
                offset +=len;
                length -= len;
            }
            return offset;
        }

        size_t Connection::writeFixSize(ByteArray::ptr byte_array, size_t length) {
            size_t offset = 0;
            while (length > 0) {
                size_t len = write(byte_array, length);
                if (len <= 0) {
                    break;
                }
                offset +=len;
                length -= len;
            }
            return offset;
        }
    }
}