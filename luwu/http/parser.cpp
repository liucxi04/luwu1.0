//
// Created by liucxi on 2022/11/23.
//

#include "parser.h"
#include <cstring>

namespace luwu {
    namespace http {
        // http 请求报文 --- 开始解析
        static int on_request_message_begin_cb(http_parser *parser) {
            return 0;
        }

        // http 请求报文 --- url 解析完成之后
        static int on_request_url_cb(http_parser *parser, const char *buf, size_t len) {
            auto *p = static_cast<HttpRequestParser *>(parser->data);
            http_parser_url url_parser{};
            http_parser_url_init(&url_parser);

            int rt = http_parser_parse_url(buf, len, 0, &url_parser);
            if (rt != 0) {
                return 1;
            }

            if (url_parser.field_set & (1 << UF_PATH)) {
                p->getData()->setPath(std::string(buf + url_parser.field_data[UF_PATH].off,
                                                  url_parser.field_data[UF_PATH].len));
            }
            if (url_parser.field_set & (1 << UF_QUERY)) {
                p->getData()->setQuery(std::string(buf + url_parser.field_data[UF_QUERY].off,
                                                   url_parser.field_data[UF_QUERY].len));
            }
            if (url_parser.field_set & (1 << UF_FRAGMENT)) {
                p->getData()->setFragment(std::string(buf + url_parser.field_data[UF_FRAGMENT].off,
                                                      url_parser.field_data[UF_FRAGMENT].len));
            }
            return 0;
        }

        // http 请求报文 --- 无用，本回调只用于响应报文
        static int on_request_status_cb(http_parser *parser, const char *buf, size_t len) {
            return 0;
        }

        // http 请求报文 --- 头部字段名称解析完成之后
        static int on_request_header_filed_cb(http_parser *parser, const char *buf, size_t len) {
            auto *p = static_cast<HttpRequestParser *>(parser->data);
            p->setFiled(std::string(buf, len));
            return 0;
        }

        // http 请求报文 --- 头部字段值解析完成之后
        static int on_request_header_value_cb(http_parser *parser, const char *buf, size_t len) {
            auto *p = static_cast<HttpRequestParser *>(parser->data);
            p->getData()->setHeader(p->getFiled(), std::string(buf, len));
            return 0;
        }

        // http 请求报文 --- 请求头解析完成之后
        static int on_request_headers_complete_cb(http_parser *parser) {
            auto *p = static_cast<HttpRequestParser *>(parser->data);
            auto method = static_cast<HttpMethod>(parser->method);
            p->getData()->setMethod(method > TRACE ? OTHER : method);
            p->getData()->setVersion(parser->http_major << 0x4 | parser->http_minor);
            return 0;
        }

        // http 请求报文 --- 请求体解析完成之后
        static int on_request_body_cb(http_parser *parser, const char *buf, size_t len) {
            auto *p = static_cast<HttpRequestParser *>(parser->data);
            p->getData()->appendBody(std::string(buf, len));
            return 0;
        }

        // http 请求报文 --- 解析完成
        static int on_request_message_complete_cb(http_parser *parser) {
            auto *p = static_cast<HttpRequestParser *>(parser->data);
            p->setFinished(true);
            return 0;
        }

        // http 请求报文 --- 分段头部开始
        static int on_request_chuck_header_cb(http_parser *parser) {
            return 0;
        }

        // http 请求报文 --- 分段头部结束
        static int on_request_chuck_complete_cb(http_parser *parser) {
            return 0;
        }

        static http_parser_settings http_request_parser = {
                .on_message_begin    = on_request_message_begin_cb,
                .on_url              = on_request_url_cb,
                .on_status           = on_request_status_cb,
                .on_header_field     = on_request_header_filed_cb,
                .on_header_value     = on_request_header_value_cb,
                .on_headers_complete = on_request_headers_complete_cb,
                .on_body             = on_request_body_cb,
                .on_message_complete = on_request_message_complete_cb,
                .on_chunk_header     = on_request_chuck_header_cb,
                .on_chunk_complete   = on_request_chuck_complete_cb
        };

        HttpRequestParser::HttpRequestParser()
            : finished_(false), error_(0), data_(new HttpRequest) {
            http_parser_init(&parser_, HTTP_REQUEST);
            parser_.data = this;
        }

        size_t HttpRequestParser::execute(char *data, size_t len) {
            size_t n = http_parser_execute(&parser_, &http_request_parser, data, len);
            if (parser_.upgrade) {
                data_->setWebsocket(true);
            } else if (static_cast<int>(parser_.http_errno) != 0) {
                setError(static_cast<int>(parser_.http_errno));
            } else {
                if (n < len) {
                    // 还没有解析完，把没有解析的部分移到最前面
                    memmove(data, data + n, len - n);
                }
            }
            return n;
        }

        // http 响应报文 --- 开始解析
        static int on_response_message_begin_cb(http_parser *parser) {
            return 0;
        }

        // http 响应报文 --- 无用，本回调只用于请求报文
        static int on_response_url_cb(http_parser *parser, const char *buf, size_t len) {
            return 0;
        }

        // http 响应报文 --- 响应状态解析完成之后
        static int on_response_status_cb(http_parser *parser, const char *buf, size_t len) {
            auto *p = static_cast<HttpResponseParser *>(parser->data);
            p->getData()->setStatus(static_cast<HttpStatus>(parser->status_code));
            return 0;
        }

        // http 响应报文 --- 头部字段名称解析完成之后
        static int on_response_header_filed_cb(http_parser *parser, const char *buf, size_t len) {
            auto *p = static_cast<HttpResponseParser *>(parser->data);
            p->setFiled(std::string(buf, len));
            return 0;
        }

        // http 响应报文 --- 头部字段值解析完成之后
        static int on_response_header_value_cb(http_parser *parser, const char *buf, size_t len) {
            auto *p = static_cast<HttpResponseParser *>(parser->data);
            p->getData()->setHeader(p->getFiled(), std::string(buf, len));
            return 0;
        }

        // http 响应报文 --- 响应头解析完成之后
        static int on_response_headers_complete_cb(http_parser *parser) {
            auto *p = static_cast<HttpResponseParser *>(parser->data);
            p->getData()->setVersion(parser->http_major << 0x4 | parser->http_minor);
            return 0;
        }

        // http 请求报文 --- 请求体解析完成之后
        static int on_response_body_cb(http_parser *parser, const char *buf, size_t len) {
            auto *p = static_cast<HttpResponseParser *>(parser->data);
            p->getData()->appendBody(std::string(buf, len));
            return 0;
        }

        // http 请求报文 --- 解析完成
        static int on_response_message_complete_cb(http_parser *parser) {
            auto *p = static_cast<HttpResponseParser *>(parser->data);
            p->setFinished(true);
            return 0;
        }

        // http 请求报文 --- 分段头部开始
        static int on_response_chuck_header_cb(http_parser *parser) {
            return 0;
        }

        // http 请求报文 --- 分段头部结束
        static int on_response_chuck_complete_cb(http_parser *parser) {
            return 0;
        }

        static http_parser_settings http_response_parser = {
                .on_message_begin    = on_response_message_begin_cb,
                .on_url              = on_response_url_cb,
                .on_status           = on_response_status_cb,
                .on_header_field     = on_response_header_filed_cb,
                .on_header_value     = on_response_header_value_cb,
                .on_headers_complete = on_response_headers_complete_cb,
                .on_body             = on_response_body_cb,
                .on_message_complete = on_response_message_complete_cb,
                .on_chunk_header     = on_response_chuck_header_cb,
                .on_chunk_complete   = on_response_chuck_complete_cb
        };

        HttpResponseParser::HttpResponseParser()
            : finished_(false), error_(0), data_(new HttpResponse) {
            http_parser_init(&parser_, HTTP_RESPONSE);
            parser_.data = this;
        }

        size_t HttpResponseParser::execute(char *data, size_t len) {
            size_t n = http_parser_execute(&parser_, &http_response_parser, data, len);
            if (static_cast<int>(parser_.http_errno) != 0) {
                setError(static_cast<int>(parser_.http_errno));
            } else {
                if (n < len) {
                    // 还没有解析完，把没有解析的部分移到最前面
                    memmove(data, data + n, len - n);
                }
            }
            return n;
        }
    }
}