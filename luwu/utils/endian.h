//
// Created by liucxi on 2022/11/19.
//

#ifndef LUWU_ENDIAN_H
#define LUWU_ENDIAN_H

#include <byteswap.h>
#include <type_traits>
#include <cstdint>

namespace luwu {
    template<typename T>
    typename std::enable_if<sizeof(T) == sizeof(uint64_t), T>::type
    byteSwap(T value) {
        return static_cast<T>(bswap_64(static_cast<uint64_t>(value)));
    }

    template<typename T>
    typename std::enable_if<sizeof(T) == sizeof(uint32_t), T>::type
    byteSwap(T value) {
        return static_cast<T>(bswap_32(static_cast<uint32_t>(value)));
    }

    template<typename T>
    typename std::enable_if<sizeof(T) == sizeof(uint16_t), T>::type
    byteSwap(T value) {
        return static_cast<T>(bswap_16(static_cast<uint16_t>(value)));
    }

#if BYTE_ORDER == BIG_ENDIAN
    template<typename T>
    T onBigEndian(T t) {
        return t;
    }

    template<typename T>
    T onLittleEndian(T t) {
        return byteSwap(t);
    }
#else
    template<typename T>
    T onBigEndian(T t) {
        return byteSwap(t);
    }

    template<typename T>
    T onLittleEndian(T t) {
        return t;
    }
#endif
}

#endif //LUWU_ENDIAN_H
