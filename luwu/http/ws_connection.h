//
// Created by liucxi on 2022/11/25.
//

#ifndef LUWU_WS_CONNECTION_H
#define LUWU_WS_CONNECTION_H

#include <utility>

#include "http_connection.h"

namespace luwu {
    namespace http {
#pragma pack(1)
        /**
         * @brief WS 数据帧头部
         */
        struct WSFrameHead {
            enum OPCODE {
                CONTINUE = 0x0,
                TEXT_FRAME = 0x1,
                BIN_FRAME = 0x2,
                CLOSE = 0x8,
                PING = 0x9,
                PONG = 0xA,
            };

            bool fin_:1;
            bool rsv1_:1;
            bool rsv2_:1;
            bool rsv3_:1;
            uint8_t opcode_:4;
            bool mask_:1;
            uint8_t payload_:7;

            /**
             * @brief 根据 data 初始化成员变量
             * @param data 原始数据
             */
            void init(const uint8_t *data);

            /**
             * @brief 根据所给信息构造原始数据
             * @param data 构造出的原始数据
             * @param fin 是否是消息的最后一个分片
             * @param opcode 操作代码
             * @param size 数据长度
             */
            static void generate(uint8_t *data, bool fin, uint8_t opcode, uint64_t &size);

            /**
             * @brief 将数据帧头部信息格式化为字符串
             * @return 字符串
             */
            std::string toString() const;
        };
#pragma pack()

        /**
         * @brief WS 消息
         */
        class WSFrameMessage {
        public:
            using ptr = std::shared_ptr<WSFrameMessage>;

            /**
             * @brief 构造函数
             * @param opcode 操作代码
             * @param data 客户端发送的数据
             */
            explicit WSFrameMessage(uint8_t opcode = 0, std::string data = "") : opcode_(opcode), data_(std::move(data)) { }

            // region # Getter and Setter
            uint8_t getOpcode() const { return opcode_; }

            void setOpcode(uint8_t opcode) { opcode_ = opcode; }

            std::string &getData() { return data_; }

            const std::string &getData() const { return data_; }

            void setData(const std::string &data) { data_ = data; }
            // endregion

        private:
            /// 操作代码
            uint8_t opcode_;
            /// 客户端发送的数据
            std::string data_;
        };

        /**
         * @brief WS 连接
         */
        class WSConnection : public HttpConnection {
        public:
            using ptr = std::shared_ptr<WSConnection>;

            /**
             * @brief 构造函数
             * @param socket ws 连接所持有的 socket
             */
            explicit WSConnection(Socket::ptr socket) : HttpConnection(std::move(socket)) { }

            /**
             * @brief 处理握手请求
             * @return http 请求
             */
            HttpRequest::ptr handleShake();

            /**
             * @brief 接收 ws 消息
             * @return ws 消息
             */
            WSFrameMessage::ptr recvMessage();

            /**
             * @brief 发送 ws 消息
             * @param msg ws 消息
             * @param fin 是否是消息的最后一帧
             * @return 成功发送的消息的长度
             */
            uint64_t sendMessage(const WSFrameMessage::ptr& msg, bool fin = true);

            /**
             * @brief ping 操作
             * @return 成功发送的消息的长度
             */
            size_t ping();

            /**
             * @brief pong 操作
             * @return 成功发送的消息的长度
             */
            size_t pong();
        };

    }
}

#endif //LUWU_WS_CONNECTION_H
