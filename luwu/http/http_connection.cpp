//
// Created by liucxi on 2022/11/24.
//

#include "http_connection.h"
#include "http_parser.h"

namespace luwu {
    namespace http {
        HttpRequest::ptr HttpConnection::recvRequest() {
            HttpRequestParser::ptr parser(new HttpRequestParser);
            uint64_t buffer_size = HttpRequestParser::GetHttpRequestBufferSize();
            std::string buffer;
            buffer.resize(buffer_size);
            size_t offset = 0;                  // 上次剩下没解析的
            // 避免一次性读不完
            while (true) {
                size_t len = read(&buffer[offset], buffer_size - offset);
                if (len < 0) {
                    break;
                }
                len += offset;
                size_t n = parser->execute(&buffer[0], len);
                if (parser->getError() != 0) {
                    break;
                }
                offset = len - n;
                if (parser->isFinished()) {
                    parser->getData()->initState();
                    return parser->getData();
                }
            }
            return nullptr;
        }

        size_t HttpConnection::sendResponse(const HttpResponse::ptr& rsp) {
            std::string buffer = rsp->toString();
            size_t send_bytes = 0, left_bytes = buffer.size();
            // 避免一次性写不完
            while (left_bytes > 0) {
                size_t len = write(&buffer[send_bytes], left_bytes);
                if (len <= 0) {
                    return send_bytes;
                }
                send_bytes += len;
                left_bytes -= len;
            }
            return send_bytes;
        }

        size_t HttpConnection::read(void *buffer, size_t length) {
            if (!isConnected()) {
                return -1;
            }
            return socket_->recv(buffer, length);
        }

        size_t HttpConnection::read(ByteArray::ptr byte_array, size_t length) {
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

        size_t HttpConnection::write(const void *buffer, size_t length) {
            if (!isConnected()) {
                return -1;
            }
            return socket_->send(buffer, length);
        }

        size_t HttpConnection::write(ByteArray::ptr byte_array, size_t length) {
            if (!isConnected()) {
                return -1;
            }
            std::string buffer;
            buffer.resize(length);
            byte_array->read(&buffer[0], length);
            size_t len = socket_->send(buffer.data(), buffer.size());
            return len;
        }
    }
}