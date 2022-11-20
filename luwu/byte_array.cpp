//
// Created by liucxi on 2022/11/20.
//

#include "byte_array.h"

namespace luwu {

    ByteArray::ByteArray(size_t size)
        : data_(size)
        , reader_index_(0), writer_index_(0)
        , endian_(BIG_ENDIAN) {
    }

    void ByteArray::writeDouble(double val) {
        uint64_t v = 0;
        memcpy(&v, &val, sizeof val);
        writeInt<uint64_t>(v);
    }

    void ByteArray::writeString(const std::string &val) {
        uint64_t size = val.size();                         // 写字符串前先写入字符串长度
        writeInt<uint64_t>(size);
        write(val.c_str(), val.size());
    }

    void ByteArray::write(const void *buf, size_t size) {
        ensureCapacity(size);
        std::copy(static_cast<const char *>(buf),
                  static_cast<const char *>(buf) + size,
                  &*data_.begin() + writer_index_);
        writer_index_ += size;
    }

    double ByteArray::readDouble() {
        auto v = readInt<uint64_t>();
        double val = 0;
        memcpy(&val, &v, sizeof v);
        return val;
    }

    std::string ByteArray::readString() {
        auto size = readInt<uint64_t>();        // 读字符串前先读取字符串长度
        std::string val;
        val.resize(size);
        read(&val[0], size);
        return val;
    }

    void ByteArray::read(void *buf, size_t size, bool peeks) {
        if (size > getReadableSize()) {
            throw std::out_of_range("not enough readable data");
        }
        std::copy(&*data_.begin() + reader_index_,
                  &*data_.begin() + reader_index_ + size,
                  static_cast<char *>(buf));
        if (!peeks) {
            reader_index_ += size;
        }
    }

    void ByteArray::ensureCapacity(size_t size) {
        if (getWriteableSize() >= size) {
            return;
        }

        if (getWriteableSize() + getPrependSize() < size) {
            // 可写区域加上已经读过失效的区域都不够，需要新开辟空间
            // 开辟新空间时就不再考虑已经读过失效的区域
            data_.resize(writer_index_ + size);
        } else {
            // 可写区域加上已经读过失效的区域够用，只需要将可读区域移动到最前面即可
            size_t readable = getReadableSize();
            std::copy(&*data_.begin() + reader_index_,
                      &*data_.begin() + writer_index_,
                      &*data_.begin());
            reader_index_ = 0;
            writer_index_ = readable;
        }
    }
}