//
// Created by liucxi on 2022/11/21.
//

#include "message.h"
#include <sstream>
#include <cstring>
#include <algorithm>
#include "utils/util.h"

namespace luwu {

    std::string http::HttpMethodToString(http::HttpMethod method) {
#define XX(name, m) if(method == m) { return #name; }
        XX(DELETE, DELETE)
        XX(GET, GET)
        XX(HEAD, HEAD)
        XX(POST, POST)
        XX(PUT, PUT)
        XX(CONNECT, CONNECT)
        XX(OPTIONS, OPTIONS)
        XX(TRACE, TRACE)
#undef XX
        return "OTHER";
    }

    std::string http::HttpStatusToString(http::HttpStatus status) {
#define XX(name, s) if(status == s) { return #name; }
        XX(SWITCHING_PROTOCOLS, SWITCHING_PROTOCOLS)
        XX(OK, OK)
        XX(NO_CONTENT, NO_CONTENT)
        XX(PARTIAL_CONTENT, PARTIAL_CONTENT)
        XX(MOVED_PERMANENTLY, MOVED_PERMANENTLY)
        XX(FOUND, FOUND)
        XX(SEE_OTHER, SEE_OTHER)
        XX(NOT_MODIFIED, NOT_MODIFIED)
        XX(TEMPORARY_REDIRECT, TEMPORARY_REDIRECT)
        XX(BAD_REQUEST, BAD_REQUEST)
        XX(UNAUTHORIZED, UNAUTHORIZED)
        XX(PAYMENT_REQUIRED, PAYMENT_REQUIRED)
        XX(FORBIDDEN, FORBIDDEN)
        XX(NOT_FOUND, NOT_FOUND)
        XX(INTERNAL_SERVER_ERROR, INTERNAL_SERVER_ERROR)
        XX(NOT_IMPLEMENTED, NOT_IMPLEMENTED)
        XX(SERVICE_UNAVAILABLE, SERVICE_UNAVAILABLE)
#undef XX
        return "OTHER_STATUS";
    }

    namespace http {
        HttpRequest::HttpRequest(HttpMethod method, uint8_t version, bool close)
            : method_(method), version_(version), close_(close) , step_(0), websocket_(false) {
        }

        void HttpRequest::initState() {
            std::string conn = getHeader("Connection");
            if (strcasecmp(conn.c_str(), "keep-alive") == 0) {
                close_ = false;
            }
        }

        const std::string &HttpRequest::getHeader(const std::string &key, const std::string &default_value) const {
            auto it = headers_.find(key);
            return it == headers_.end() ? default_value : it->second;
        }

        const std::string &HttpRequest::getParam(const std::string &key, const std::string &default_value) {
            initQueryParam();
            initBodyParam();
            auto it = params_.find(key);
            return it == params_.end() ? default_value : it->second;
        }

        const std::string &HttpRequest::getCookie(const std::string &key, const std::string &default_value) {
            initCookies();
            auto it = cookies_.find(key);
            return it == cookies_.end() ? default_value : it->second;
        }

        std::ostream &HttpRequest::dump(std::ostream &os) const {
            os << HttpMethodToString(method_) << " " << path_
               << (query_.empty() ? "" : "?") << query_
               << (fragment_.empty() ? "" : "#") << fragment_
               << " HTTP/" << static_cast<uint32_t>(version_ >> 4) << "." << static_cast<uint32_t>(version_ & 0x0f)
               << "\r\n";

            for (auto &head : headers_) {
                os << head.first << ": " << head.second << "\r\n";
            }

            os << "\r\n";

            if (!body_.empty()) {
                os << body_;
            } else {
                os << "\r\n";
            }
            return os;
        }

        std::string HttpRequest::toString() const {
            std::stringstream ss;
            dump(ss);
            return ss.str();
        }

        // 此处需要对 url 进行解码
#define PARSE_PARAM(str, data, flag)                                \
    size_t pos = 0;                                                 \
    do {                                                            \
        size_t last = pos;                                          \
        pos = (str).find('=', pos);                                 \
        if (pos == std::string::npos) {                             \
            break;                                                  \
        }                                                           \
        size_t key = pos;                                           \
        pos = (str).find((flag), pos);                              \
        (data)[urlDecode(trim((str).substr(last, key - last)))] = urlDecode(trim((str).substr(key + 1, pos - key - 1)));\
        if (pos == std::string::npos) {                             \
            break;                                                  \
        }                                                           \
        ++pos;                                                      \
    } while (true);

        void HttpRequest::initQueryParam() {
            if (step_ & 0x1) {
                return;
            }
            PARSE_PARAM(query_, params_, '&');
            step_ |= 0x1;
        }

        void HttpRequest::initBodyParam() {
            if (step_ & 0x2) {
                return;
            }
            std::string content_type = getHeader("Content-Type");
            // 只有 "application/x-www-form-urlencoded" 格式请求体里才会携带参数
            if (strcasecmp(content_type.c_str(), "application/x-www-form-urlencoded") == 0) {
                PARSE_PARAM(body_, params_, '&');
            }
            step_ |= 0x2;
        }

        void HttpRequest::initCookies() {
            if (step_ & 0x4) {
                return;
            }
            std::string cookie = getHeader("Cookie");
            if (cookie.empty()) {
                step_ |= 0x4;
                return;
            }
            PARSE_PARAM(cookie, cookies_, ';');
            step_ |= 0x4;
        }

        HttpResponse::HttpResponse(uint8_t version, bool close)
            : version_(version), status_(HttpStatus::OK), close_(close), websocket_(false) {
        }

        void HttpResponse::initState() {
            std::string conn = getHeader("Connection");
            if (strcasecmp(conn.c_str(), "keep-alive") == 0) {
                close_ = false;
            }
        }

        const std::string &HttpResponse::getHeader(const std::string &key, const std::string &default_value) const {
            auto it = headers_.find(key);
            return it == headers_.end() ? default_value : it->second;
        }

        void HttpResponse::setCookie(const std::string &key, const std::string &val, time_t expired,
                                const std::string &path, const std::string &domain, bool secure) {
            std::stringstream ss;
            ss << key << "=" << val;
            if (expired > 0) {
                ss << ";expires=" << formatTime(expired, "%a, %d %b %Y %H:%M:%S") << " GMT";
            }
            if (!domain.empty()) {
                ss << ";domain=" << domain;
            }
            if (!path.empty()) {
                ss << ";path=" << path;
            }
            if (secure) {
                ss << ";secure";
            }
            setHeader("Set-Cookie", ss.str());
        }

        void HttpResponse::setRedirect(const std::string &uri) {
            status_ = HttpStatus::FOUND;
            setHeader("Location", uri);
        }

        std::ostream &HttpResponse::dump(std::ostream &os) const {
            os << "HTTP/" << static_cast<uint32_t>(version_ >> 4) << "." << static_cast<uint32_t>(version_ & 0x0f)
               << " " << static_cast<uint32_t>(status_)
               << " " << HttpStatusToString(status_)
               << "\r\n";

            for (auto &head : headers_) {
                os << head.first << ": " << head.second << "\r\n";
            }

            os << "\r\n";

            if (!body_.empty()) {
                os << body_;
            } else {
                os << "\r\n";
            }
            return os;
        }

        std::string HttpResponse::toString() const {
            std::stringstream ss;
            dump(ss);
            return ss.str();
        }
    }
}