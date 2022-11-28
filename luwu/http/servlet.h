//
// Created by liucxi on 2022/11/24.
//

#ifndef LUWU_SERVLET_H
#define LUWU_SERVLET_H

#include <memory>
#include <utility>
#include <functional>

#include "message.h"
#include "http_connection.h"
#include "ws_connection.h"
#include "../utils/mutex.h"

namespace luwu {
    namespace http {
        /**
         * @brief Servlet，抽象类
         */
        class ServletBase {
        public:
            using ptr = std::shared_ptr<ServletBase>;

            /**
             * @brief 构造函数
             * @param name 名称
             */
            explicit ServletBase(std::string name) : name_(std::move(name)) {};

            /**
             * @brief 虚析构函数
             */
            virtual ~ServletBase() = default;

            virtual int handle(HttpRequest::ptr req, HttpResponse::ptr rsp, HttpConnection::ptr conn) = 0;

            virtual int handle(HttpRequest::ptr req, WSFrameMessage::ptr mag, WSConnection::ptr conn) = 0;

            const std::string &getName() const { return name_; }

        private:
            /// servlet 名称
            std::string name_;
        };

        /**
         * @brief http Servlet
         */
        class HttpServlet : public ServletBase {
        public:
            using ptr = std::shared_ptr<HttpServlet>;
            using handle_func = std::function<int(HttpRequest::ptr req, HttpResponse::ptr rsp, HttpConnection::ptr session)>;

            /**
             * @brief 构造函数
             * @param name servlet 名称
             * @param func 业务处理函数
             */
            HttpServlet(std::string name, handle_func func);

            /**
             * @brief http servlet 业务处理函数
             * @param req http 请求
             * @param rsp http 响应
             * @param session http 连接
             * @return 处理结果，-1 表示错误
             */
            int handle(HttpRequest::ptr req, HttpResponse::ptr rsp, HttpConnection::ptr session) override;

            int handle(HttpRequest::ptr req, WSFrameMessage::ptr mag, WSConnection::ptr conn) override { return 0; }

        private:
            /// 业务处理函数
            handle_func func_;
        };

        /**
         * @brief not found Servlet
         */
        class ServletNotFound : public ServletBase {
        public:
            using ptr = std::shared_ptr<ServletNotFound>;

            /**
             * @brief 构造函数
             * @param content 需要展示的内容
             */
            explicit ServletNotFound() : ServletBase("NotFoundServlet") { }

            /**
             * @brief servlet 业务处理函数
             * @param req http 请求
             * @param rsp http 响应
             * @param session http 连接
             * @return 处理结果，-1 表示错误
             */
            int handle(HttpRequest::ptr req, HttpResponse::ptr rsp, HttpConnection::ptr session) override;

            int handle(HttpRequest::ptr req, WSFrameMessage::ptr mag, WSConnection::ptr conn) override { return 0; }
        };

        /**
         * @brief websocket servlet
         */
        class WSServlet : public ServletBase {
        public:
            using ptr = std::shared_ptr<WSServlet>;
            using handle_func = std::function<int(HttpRequest::ptr req, WSFrameMessage::ptr msg, WSConnection::ptr conn)>;

            /**
             * @brief 构造函数
             * @param name servlet 名称
             * @param handel 业务处理函数
             */
            WSServlet(std::string name, handle_func handel);

            /**
             * @brief ws servlet 业务处理函数
             * @param req http 请求
             * @param msg ws 消息
             * @param conn ws 连接
             * @return 处理结果，-1 表示错误
             */
            int handle(HttpRequest::ptr req, WSFrameMessage::ptr msg, WSConnection::ptr conn) override;

            int handle(HttpRequest::ptr req, HttpResponse::ptr rsp, HttpConnection::ptr conn) override { return 0; }

        private:
            /// 业务处理函数
            handle_func handel_;
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

            int handle(HttpRequest::ptr req, HttpResponse::ptr rsp, HttpConnection::ptr conn);

            int handle(HttpRequest::ptr req, WSFrameMessage::ptr mag, WSConnection::ptr conn);

            ServletBase::ptr getExactServlet(const std::string &uri);

            ServletBase::ptr getFuzzyServlet(const std::string &uri);

            ServletBase::ptr getMatchedServlet(const std::string &uri);

            // region Getter and Setter
            void addExactServlet(const std::string &uri, ServletBase::ptr servlet);

            void addExactHttpServlet(const std::string &uri, HttpServlet::handle_func func);

            void addExactWSServlet(const std::string &uri, WSServlet::handle_func func);

            void addFuzzyServlet(const std::string &uri, ServletBase::ptr servlet);

            void addFuzzyHttpServlet(const std::string &uri, HttpServlet::handle_func func);

            void addFuzzyWSServlet(const std::string &uri, WSServlet::handle_func func);

            void delExactServlet(const std::string &uri);

            void delFuzzyServlet(const std::string &uri);

            const std::string &getName() const { return name_; }

            const ServletNotFound::ptr &getDefaultServlet() const { return default_; }

            void setDefaultServlet(ServletNotFound::ptr servlet) { default_ = std::move(servlet); }
            // endregion
        private:
            /// 读写锁
            RWMutex mutex_;
            /// 分发器名称
            std::string name_;
            /// 精准匹配的 servlet
            std::unordered_map<std::string, ServletBase::ptr> exact_;
            /// 模糊匹配的 servlet
            std::vector<std::pair<std::string, ServletBase::ptr>> fuzzy_;
            /// 默认的 servlet，匹配不上时使用
            ServletNotFound::ptr default_;
        };
    }
}

#endif //LUWU_SERVLET_H
