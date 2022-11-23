//
// Created by liucxi on 2022/11/21.
//

#ifndef LUWU_HTTP_H
#define LUWU_HTTP_H

#include <memory>
#include <vector>
#include <unordered_map>

namespace luwu {
    namespace http {
        enum HttpMethod {
            DELETE = 0,
            GET = 1,
            HEAD = 2,
            POST = 3,
            PUT = 4,
            CONNECT = 5,
            OPTIONS = 6,
            TRACE = 7,
            OTHER = 100,
        };

        enum HttpStatus {
            OK = 200,
            NO_CONTENT = 204,
            PARTIAL_CONTENT = 206,

            MOVED_PERMANENTLY = 301,
            FOUND = 302,
            SEE_OTHER = 303,
            NOT_MODIFIED = 304,
            TEMPORARY_REDIRECT = 307,

            BAD_REQUEST = 400,
            UNAUTHORIZED = 401,
            PAYMENT_REQUIRED = 402,
            FORBIDDEN = 403,
            NOT_FOUND = 404,

            INTERNAL_SERVER_ERROR = 500,
            NOT_IMPLEMENTED = 501,
            SERVICE_UNAVAILABLE = 503,

            OTHER_STATUS = 1000,
        };

        static std::string HttpMethodToString(HttpMethod method);

        static std::string HttpStatusToString(HttpStatus status);

        class HttpRequest {
        public:
            using ptr = std::shared_ptr<HttpRequest>;
            using MapType = std::unordered_map<std::string, std::string>;

            explicit HttpRequest(HttpMethod method = HttpMethod::GET, uint8_t version = 0x11, bool close = true);

            // region # Getter and Setter
            HttpMethod getMethod() const { return method_; }

            void setMethod(HttpMethod method) { method_ = method; }

            const std::string &getPath() const { return path_; }

            void setPath(const std::string &path) { path_ = path; }

            const std::string &getQuery() const { return query_; }

            void setQuery(const std::string &query) { query_ = query; }

            const std::string &getFragment() const { return fragment_; }

            void setFragment(const std::string &fragment) { fragment_ = fragment; }

            uint8_t getVersion() const { return version_; }

            void setVersion(uint8_t version) { version_ = version; }

            const std::string &getBody() const { return body_; }

            void setBody(const std::string &body) { body_ = body; }

            void appendBody(const std::string &body) { body_.append(body); }

            uint8_t getStep() const { return step_; }

            void setStep(uint8_t step) { step_ = step; }

            const std::string &getHeader(const std::string &key, const std::string &default_value = "") const;

            const std::string &getParam(const std::string &key, const std::string &default_value = "");

            const std::string &getCookie(const std::string &key, const std::string &default_value = "");

            void setHeader(const std::string &key, const std::string &value) { headers_[key] = value; }

            void setParam(const std::string &key, const std::string &value) { params_[key] = value; }

            void setCookie(const std::string &key, const std::string &value) { cookies_[key] = value; }

            void delHeader(const std::string &key) { headers_.erase(key); }

            void delParam(const std::string &key) { params_.erase(key); }

            void delCookie(const std::string &key) { cookies_.erase(key); }

            bool isClose() const { return close_; }

            void setClose(bool close) { close_ = close; }
            // endregion

            std::ostream &dump(std::ostream &os) const;

            std::string toString() const;

        private:
            void initQueryParam();

            void initBodyParam();

            void initCookies();

        private:
            /// 请求方式
            HttpMethod method_;
            /// url 路径
            std::string path_;
            /// url 查询
            std::string query_;
            /// url 信息片段
            std::string fragment_;
            /// 协议版本
            uint8_t version_;
            /// 请求头
            MapType headers_;
            /// 请求体
            std::string body_;

            /// param 与 cookie 解析状态：0 未解析、1 解析 QueryParam、2 解析 BodyParam、4 解析Cookies
            uint8_t step_;
            /// 请求参数
            MapType params_;
            /// 请求 cookie，可以有多个
            MapType cookies_;
            /// 是否长连接，是否自动关闭
            bool close_;
        };

        class HttpResponse {
        public:
            using ptr = std::shared_ptr<HttpResponse>;
            using MapType = std::unordered_map<std::string, std::string>;

            explicit HttpResponse(uint8_t version = 0x11, bool close = true);

            // region # Getter and Setter
            uint8_t getVersion() const {return version_; }

            void setVersion(uint8_t version) { version_ = version; }

            HttpStatus getStatus() const { return status_; }

            void setStatus(HttpStatus status) { status_ = status; }

            const std::string &getHeader(const std::string &key, const std::string &default_value = "") const;

            void setHeader(const std::string &key, const std::string &value) { headers_[key] = value; }

            void delHeader(const std::string &key) { headers_.erase(key); }

            const std::string &getBody() const { return body_; }

            void setBody(const std::string &body) { body_ = body; }

            void appendBody(const std::string &body) { body_.append(body); }

            bool isClose() const { return close_; }

            void setClose(bool close) { close_ = close; }

            void setCookie(const std::string &key, const std::string &val, time_t expired = 0,
                           const std::string &path = "", const std::string &domain = "", bool secure = false);

            void setRedirect(const std::string &uri);
            // endregion

            std::ostream &dump(std::ostream &os) const;

            std::string toString() const;

        private:
            /// 协议版本
            uint8_t version_;
            /// 响应状态
            HttpStatus status_;
            /// 响应头
            MapType headers_;
            /// 响应体
            std::string body_;
            /// 是否长连接，是否自动关闭
            bool close_;
        };
    }
}
#endif //LUWU_HTTP_H
