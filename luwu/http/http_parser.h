//
// Created by liucxi on 2022/11/23.
//

#ifndef LUWU_HTTP_PARSER_H
#define LUWU_HTTP_PARSER_H

#include "http.h"
#include "http-parser/http_parser.h"

namespace luwu {
    namespace http {
        /**
         * @brief HTTP 请求解析器
         */
        class HttpRequestParser {
        public:
            using ptr = std::shared_ptr<HttpRequestParser>;

            static uint64_t GetHttpRequestBufferSize();

            static uint64_t GetHttpRequestMaxBodySize();
            /**
             * @brief 构造函数
             */
            HttpRequestParser();

            /**
             * @brief 执行解析
             * @param data 需要解析的 HTTP 请求报文
             * @param len 报文长度
             * @return 成功解析的报文的长度
             */
            size_t execute(char *data, size_t len);

            // region # Getter
            bool isFinished() const { return finished_; }

            void setFinished(bool finished) { finished_ = finished; }

            int getError() const { return error_; }

            void setError(int error) { error_ = error; }

            const std::string &getFiled() const { return filed_; }

            void setFiled(const std::string &filed) { filed_ = filed; }

            const http_parser &getParser() const { return parser_; }

            HttpRequest::ptr getData() { return data_; }
            // endregion

        private:
            /// HTTP 请求解析是否完成
            bool finished_;
            /// HTTP 请求解析是否出错
            int error_;
            /// 临时存储请求头名称
            std::string filed_;
            /// http_parser 解析器
            http_parser parser_{};
            /// 解析结果构造出的 HttpRequest 对象
            HttpRequest::ptr data_;
        };

        /**
         * @brief HTTP 响应解析器
         */
        class HttpResponseParser {
        public:
            using ptr = std::shared_ptr<HttpResponseParser>;

            static uint64_t GetHttpResponseBufferSize();

            static uint64_t GetHttpResponseMaxBodySize();

            /**
             * @brief 构造函数
             */
            HttpResponseParser();

            /**
             * @brief 执行解析
             * @param data
             * @param len
             * @return
             */
            size_t execute(char *data, size_t len);

            // region # Getter and Setter
            bool isFinished() const { return finished_; }

            void setFinished(bool finished) { finished_ = finished; }

            int getError() const { return error_; }

            void setError(int error) { error_ = error; }

            const std::string &getFiled() const { return filed_; }

            void setFiled(const std::string &filed) { filed_ = filed; }

            const http_parser &getParser() const { return parser_; }

            HttpResponse::ptr getData() { return data_; }
            // endregion

        private:
            /// http 响应解析是否完成
            bool finished_;
            /// http 响应解析是否出错
            int error_;
            /// 临时存储响应头名称
            std::string filed_;
            /// http_parser 解析器
            http_parser parser_{};
            /// 解析结果构造出的 HttpResponse 对象
            HttpResponse::ptr data_;
        };
    }
}

#endif //LUWU_HTTP_PARSER_H
