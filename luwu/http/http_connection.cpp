//
// Created by liucxi on 2022/11/24.
//

#include "http_connection.h"
#include "parser.h"

namespace luwu {
    namespace http {
        static const uint64_t s_http_request_buffer_size = 4 * 1024;
        static const uint64_t s_http_request_max_body_size = 64 * 1024 * 1024;
        static const uint64_t s_http_response_buffer_size = 4 * 1024;
        static const uint64_t s_http_response_max_body_size = 64 * 1024 * 1024;

        HttpRequest::ptr HttpConnection::recvRequest() {
            HttpRequestParser::ptr parser(new HttpRequestParser);
            uint64_t buffer_size = s_http_request_buffer_size;
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
            return writeFixSize(buffer.c_str(), buffer.size());
        }
    }
}