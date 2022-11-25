//
// Created by liucxi on 2022/11/24.
//

#ifndef LUWU_HTTP_SERVLET_H
#define LUWU_HTTP_SERVLET_H

#include <memory>
#include <utility>
#include <functional>

#include "http.h"
#include "http_connection.h"
#include "../utils/mutex.h"

namespace luwu {
    namespace http {
        /**
         * @brief Servlet，抽象类
         */
        class Servlet {
        public:
            using ptr = std::shared_ptr<Servlet>;

            /**
             * @brief 构造函数
             * @param name 名称
             */
            explicit Servlet(std::string name) : name_(std::move(name)) {};

            /**
             * @brief 虚析构函数
             */
            virtual ~Servlet() = default;

            /**
             * @brief servlet 业务处理函数
             * @param req http 请求
             * @param rsp http 响应
             * @param session http 连接
             * @return 处理结果，-1 表示错误
             */
            virtual int handle(HttpRequest::ptr req, HttpResponse::ptr rsp, HttpConnection::ptr session) = 0;

            const std::string &getName() const { return name_; }

        private:
            /// servlet 名称
            std::string name_;
        };

        /**
         * @brief 函数式 Servlet
         */
        class ServletFunction : public Servlet {
        public:
            using ptr = std::shared_ptr<ServletFunction>;
            using handle_func = std::function<int(HttpRequest::ptr req, HttpResponse::ptr rsp, HttpConnection::ptr session)>;

            /**
             * @brief 构造函数
             * @param name servlet 名称
             * @param func 业务处理函数
             */
            ServletFunction(std::string name, handle_func func);

            /**
             * @brief servlet 业务处理函数
             * @param req http 请求
             * @param rsp http 响应
             * @param session http 连接
             * @return 处理结果，-1 表示错误
             */
            int handle(HttpRequest::ptr req, HttpResponse::ptr rsp, HttpConnection::ptr session) override;

        private:
            /// 业务处理函数
            handle_func func_;
        };

        /**
         * @brief not found Servlet
         */
        class ServletNotFound : public Servlet {
        public:
            using ptr = std::shared_ptr<ServletNotFound>;

            /**
             * @brief 构造函数
             * @param content 需要展示的内容
             */
            explicit ServletNotFound() : Servlet("NotFoundServlet") { }

            /**
             * @brief servlet 业务处理函数
             * @param req http 请求
             * @param rsp http 响应
             * @param session http 连接
             * @return 处理结果，-1 表示错误
             */
            int handle(HttpRequest::ptr req, HttpResponse::ptr rsp, HttpConnection::ptr session) override;
        };

        /**
         * @brief Servlet 分发器
         */
        class ServletDispatch {
        public:
            using ptr = std::shared_ptr<ServletDispatch>;

            /**
             * @brief 构造函数
             * @param name 分发器名称
             */
            explicit ServletDispatch(std::string name);

            /**
             * @brief 业务处理函数
             * @param req http 请求
             * @param rsp http 响应
             * @param session http 连接
             * @return 处理结果，-1 表示错误
             */
            int handle(HttpRequest::ptr req, HttpResponse::ptr rsp, HttpConnection::ptr session);

            /**
             * @brief 根据 uri 找到对应的精准匹配的 servlet
             * @param uri uri
             * @return 对应的 servlet，找不到返回 nullptr
             */
            Servlet::ptr getExactServlet(const std::string &uri);

            /**
             * @brief 根据 uri 找到对应的模糊匹配的 servlet
             * @param uri uri
             * @return 对应的 servlet，找不到返回 nullptr
             */
            Servlet::ptr getFuzzyServlet(const std::string &uri);

            /**
             * @brief 根据 uri 找到对应的 servlet
             * @param uri uri
             * @return 对应的 servlet，找不到返回默认 servlet
             */
            Servlet::ptr getMatchedServlet(const std::string &uri);

            // region Getter and Setter
            void addExactServlet(const std::string &uri, Servlet::ptr servlet);

            void addExactServlet(const std::string &uri, ServletFunction::handle_func func);

            void addFuzzyServlet(const std::string &uri, Servlet::ptr servlet);

            void addFuzzyServlet(const std::string &uri, ServletFunction::handle_func func);

            void delExactServlet(const std::string &uri);

            void delFuzzyServlet(const std::string &uri);

            const std::string &getName() const { return name_; }

            const Servlet::ptr &getDefaultServlet() const { return default_; }

            void setDefaultServlet(Servlet::ptr servlet) { default_ = std::move(servlet); }
            // endregion
        private:
            /// 读写锁
            RWMutex mutex_;
            /// 分发器名称
            std::string name_;
            /// 精准匹配的 servlet
            std::unordered_map<std::string, Servlet::ptr> exact_;
            /// 模糊匹配的 servlet
            std::vector<std::pair<std::string, Servlet::ptr>> fuzzy_;
            /// 默认的 servlet，匹配不上时使用
            Servlet::ptr default_;
        };
    }
}

#endif //LUWU_HTTP_SERVLET_H
