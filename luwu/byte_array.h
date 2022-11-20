//
// Created by liucxi on 2022/11/20.
//

#ifndef LUWU_BYTE_ARRAY_H
#define LUWU_BYTE_ARRAY_H

#include <memory>
#include <vector>
#include <cstring>
#include "utils/endian.h"

namespace luwu {
    /**
     * @brief 字节数组
     */
    class ByteArray {
    public:
        using ptr = std::shared_ptr<ByteArray>;

        /**
         * @brief 构造函数
         * @param size 初始字节数组大小
         */
        explicit ByteArray(size_t size = 4096);

        /**
         * @brief 向数组中写入整数
         * @tparam T 整数类型
         * @param val 整数值
         */
        template<typename T>
        void writeInt(T val) {
            if (endian_ != BYTE_ORDER) {
                val = byteSwap(val);
            }
            write(&val, sizeof val);
        }

        /**
         * @brief 向数组中写入浮点数
         * @param val 浮点数值
         */
        void writeDouble(double val);

        /**
         * @brief 向数组中写入字符串
         * @param val 字符串
         */
        void writeString(const std::string &val);

        /**
         * @brief 向数组中写入一段缓冲区内容
         * @param buf 缓冲区
         * @param size 缓冲区大小
         */
        void write(const void *buf, size_t size);

        /**
         * @brief 从数组中读取整数
         * @tparam T 整数类型
         * @return 整数值
         */
        template<typename T>
        T readInt() {
            T val = 0;
            read(&val, sizeof val);
            if (endian_ != BYTE_ORDER) {
                val = byteSwap(val);
            }
            return val;
        }

        /**
         * @brief 从数组中读取浮点数
         * @return 浮点数值
         */
        double readDouble();

        /**
         * @brief 从数组中读取字符串
         * @return 字符串
         */
        std::string readString();

        /**
         * @brief 从数组中读取一段内容到缓冲区
         * @param buf 缓冲区
         * @param size 缓冲区大小
         * @param peeks 窥探，不改变数组
         */
        void read(void *buf, size_t size, bool peeks = false);

        /**
         * @brief 已经读取过，失效的区域大小
         * @return 区域大小
         */
        size_t getReadableSize() const { return writer_index_ - reader_index_; }

        /**
         * @brief 可写区域大小
         * @return 区域大小
         */
        size_t getWriteableSize() const { return data_.size() - writer_index_; }

        /**
         * @brief 可读区域大小
         * @return 区域大小
         */
        size_t getPrependSize() const { return reader_index_; }

    private:
        /**
         * @brief 确保可写区域足够
         * @param size 需要的可写区域大小
         */
        void ensureCapacity(size_t size);

    private:
        /// 数据指针
        std::vector<char> data_;
        /// 读数据的位置
        size_t reader_index_;
        /// 写数据的位置
        size_t writer_index_;
        /// 字节序，默认大端
        uint16_t endian_;
    };
}

#endif //LUWU_BYTE_ARRAY_H
