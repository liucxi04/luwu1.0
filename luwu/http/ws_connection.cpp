//
// Created by liucxi on 2022/11/25.
//

#include "ws_connection.h"
#include <memory>
#include <sstream>
#include <iostream>
#include "utils/util.h"

namespace luwu {
    namespace http {
        static const uint64_t s_ws_message_max_size = 1024 * 1024 * 32;

        // 在处理 websocket 数据帧头部的过程中，直接将网络数据读取到 WSFrameHead 对象中会出错，错误的数据成员为 opcode_ 和 payload_
        // 这两个都是整数类型，由于网络字节序是大端，本地字节序是小端，所以这两个成员变量的字节序不匹配导致整数值出错。
        // 在这里使用位运算一一进行赋值，这就是 init 和 generate 存在的原因
        void WSFrameHead::init(const uint8_t *data) {
            fin_ = data[0] & 0x80;
            rsv1_ = data[0] & 0x40;
            rsv2_ = data[0] & 0x20;
            rsv3_ = data[0] & 0x10;
            opcode_ = data[0] & 0x0f;
            mask_ = data[1] & 0x80;
            payload_ = data[1] & 0x7f;
        }

        void WSFrameHead::generate(uint8_t *data, bool fin, uint8_t opcode, uint64_t &size) {
            if (fin) {
                data[0] = 0x80;
            }
            data[0] |= (opcode & 0x0f);
            if (size >= 65536) {
                size = 127;
            } else if (size >= 126) {
                size = 126;
            }
            data[1] |= (size & 0x7f);
        }

        std::string WSFrameHead::toString() const {
            std::stringstream ss;
            ss << "[WSFrameHead fin = " << fin_
               << " rsv1 = " << rsv1_ << " rsv2 = " << rsv2_ << " rsv3 = " << rsv3_
               << " opcode = " << static_cast<int>(opcode_)
               << " mask = " << mask_ << " payload = " << static_cast<int>(payload_) << "]";
            return ss.str();
        }

        HttpRequest::ptr WSConnection::handelShake() {
            HttpRequest::ptr req;
            // do while (false) 避免了 if 语句的层层嵌套
            do {
                req = recvRequest();
                if (!req) {
                    break;
                }
                // http 请求升级为 websocket 请求必须携带的四个头部字段
                if (strcasecmp(req->getHeader("Connection").c_str(), "Upgrade") != 0 ||
                    strcasecmp(req->getHeader("Upgrade").c_str(), "websocket") != 0 ||
                    strcasecmp(req->getHeader("Sec-WebSocket-Version").c_str(), "13") != 0) {
                    break;
                }
                std::string key = req->getHeader("Sec-WebSocket-Key");
                if (key.empty()) {
                    break;
                }

                // 生成 Sec-WebSocket-Accept
                std::string val = key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
                std::string hash_val = sha1sum(val.c_str(), val.size());
                std::string encode_val = base64encode(hash_val.c_str(), hash_val.size());

                // 构造 http 响应
                HttpResponse::ptr rsp(new HttpResponse(req->getVersion(), req->isClose()));
                rsp->setWebsocket(true);
                rsp->setStatus(HttpStatus::SWITCHING_PROTOCOLS);
                rsp->setHeader("Upgrade", "websocket");
                rsp->setHeader("Connection", "Upgrade");
                rsp->setHeader("Sec-WebSocket-Accept", encode_val);

                sendResponse(rsp);
                return req;
            } while (false);
            return nullptr;
        }

        WSFrameMessage::ptr WSConnection::recvMessage() {
            std::string data;
            uint8_t opcode = 0;
            size_t cur_len = 0;
            WSFrameHead head{};
            do {
                uint8_t tmp[2];
                if (readFixSize(&tmp, sizeof tmp) <= 0) {
                    break;
                }
                head.init(tmp);

                if (head.opcode_ == WSFrameHead::PING) {
                    pong();
                    break;
                } else if (head.opcode_ == WSFrameHead::CONTINUE ||
                        head.opcode_ == WSFrameHead::TEXT_FRAME ||
                        head.opcode_ == WSFrameHead::BIN_FRAME) {
                    // 客户端请求必须进行过掩码操作
                    if (!head.mask_) {
                        break;
                    }

                    uint64_t length;
                    if (head.payload_ == 126) {
                        uint16_t len = 0;
                        if (readFixSize(&len, sizeof len) <= 0) {
                            break;
                        }
                        length = onBigEndian(len);
                    } else if (head.payload_ == 127) {
                        uint64_t len = 0;
                        if (readFixSize(&len, sizeof len) <= 0) {
                            break;
                        }
                        length = onBigEndian(len);
                    } else {
                        length = head.payload_;
                    }

                    if (cur_len + length >= s_ws_message_max_size) {
                        break;
                    }

                    uint8_t mask[4] = {0};
                    if (readFixSize(mask, sizeof mask) <= 0) {
                        break;
                    }
                    data.resize(cur_len + length);
                    if (readFixSize(&data[cur_len], length) <= 0) {
                        break;
                    }
                    for (uint64_t i = 0; i < length; ++i) {
                        data[cur_len + i] ^= mask[i % 4];
                    }

                    cur_len += length;
                    if (!opcode && head.opcode_ != WSFrameHead::CONTINUE) {
                        opcode = head.opcode_;
                    }

                    if (head.fin_) {
                        return std::make_shared<WSFrameMessage>(opcode, std::move(data));
                    }
                } else if (head.opcode_ == WSFrameHead::PONG || head.opcode_ == WSFrameHead::CLOSE){
                    break;
                }
            } while (true);
            return nullptr;
        }

        uint64_t WSConnection::sendMessage(const WSFrameMessage::ptr& msg, bool fin) {
            do {
                uint8_t tmp[2] = {0};
                uint64_t size = msg->getData().size();
                WSFrameHead::generate(tmp, fin, msg->getOpcode(), size);

                // 写数据帧头部
                if (writeFixSize(&tmp, sizeof tmp) <= 0) {
                    break;
                }
                // 写附加的长度
                if (size == 126) {
                    uint16_t len = msg->getData().size();
                    len = onBigEndian(len);
                    if (writeFixSize(&len, sizeof len) <= 0) {
                        break;
                    }
                } else if (size == 127) {
                    uint64_t len = msg->getData().size();
                    len = onBigEndian(len);
                    if (writeFixSize(&len, sizeof len) <= 0) {
                        break;
                    }
                }
                // 写数据
                if (writeFixSize(msg->getData().c_str(), msg->getData().size()) <= 0) {
                    break;
                }
                return size + 2;
            } while (false);
            return -1;
        }

        size_t WSConnection::ping() {
            uint8_t data[2] = {0};
            size_t size = sizeof data;
            WSFrameHead::generate(data, true, WSFrameHead::PING, size);
            return writeFixSize(&data, sizeof size);
        }

        size_t WSConnection::pong() {
            uint8_t data[2] = {0};
            size_t size = sizeof data;
            WSFrameHead::generate(data, true, WSFrameHead::PONG, size);
            return writeFixSize(&data, sizeof size);
        }
    }
}