//
// Created by liucxi on 2022/11/1.
//

#include "util.h"
#include "../fiber.h"
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <execinfo.h>
#include <sstream>
#include <iostream>
#include <openssl/sha.h>

namespace luwu {
    uint32_t getThreadId() {
        return syscall(SYS_gettid);
    }

    uint32_t getFiberId() {
        return Fiber::GetFiberId();
    }

    // 将 Logger 定义中的 create_time_ 初始化由 time(nullptr) 改为 create_time_(getElapseMs()) 基本满足要求
    // 后续有更好的办法 ？ std::chrono ？
    uint64_t getElapseMs() {
        struct timespec ts{};
        clock_gettime(CLOCK_MONOTONIC_RAW, &ts);            // 系统开始运行到现在的时间
        return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
    }

    std::string getThreadName() {
        // 系统调用要求不能超过 16 字节
        char thread_name[16];
        pthread_getname_np(pthread_self(), thread_name, 16);
        return thread_name;
    }

    void setThreadName(const std::string &name) {
        pthread_setname_np(pthread_self(), name.substr(0, 15).c_str());
    }

    uint64_t getCurrentTime() {
        struct timeval val{};
        gettimeofday(&val, nullptr);
        return val.tv_sec * 1000 + val.tv_usec / 1000;
    }

    void backtrace(std::vector<std::string> &bt, int size, int skip) {
        void **array = (void **) ::malloc(sizeof(void *) * size);
        int s = ::backtrace(array, size);

        char **strings = ::backtrace_symbols(array, size);
        if (strings == nullptr) {
            std::cerr << "backtrace_symbols error" << std::endl;
        }

        for (int i = skip; i < s; ++i) {
            bt.emplace_back(strings[i]);
        }
        ::free(array);
        ::free(strings);
    }

    std::string backtraceToString(int size, int skip, const std::string &prefix) {
        std::vector<std::string> bt;
        backtrace(bt, size, skip);

        std::stringstream ss;
        for (const auto &i: bt) {
            ss << prefix << " " << i << std::endl;
        }
        return ss.str();
    }

    std::string formatTime(time_t ts, const std::string &format) {
        struct tm tm{};
        localtime_r(&ts, &tm);
        char buf[64];
        strftime(buf, sizeof(buf), format.c_str(), &tm);
        return buf;
    }

    static const char uri_chars[256] = {
            /* 0 */
            0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 1, 1, 0,
            1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 0, 0, 0, 1, 0, 0,
            /* 64 */
            0, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 0, 0, 0, 0, 1,
            0, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 0, 0, 0, 1, 0,
            /* 128 */
            0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
            /* 192 */
            0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    };

    static const char x_digit_chars[256] = {
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,1,2,3,4,5,6,7,8,9,0,0,0,0,0,0,
            0,10,11,12,13,14,15,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,10,11,12,13,14,15,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    };

    std::string urlEncode(const std::string &str, bool space_as_plus) {
        static const char *hexdigits = "0123456789ABCDEF";
        std::string* ss = nullptr;
        const char* end = str.c_str() + str.length();
        for(const char* c = str.c_str() ; c < end; ++c) {
            if(!uri_chars[(unsigned char)(*c)]) {
                if(!ss) {
                    ss = new std::string;
                    ss->reserve(str.size() * 1.2);
                    ss->append(str.c_str(), c - str.c_str());
                }
                if(*c == ' ' && space_as_plus) {
                    ss->append(1, '+');
                } else {
                    ss->append(1, '%');
                    ss->append(1, hexdigits[(uint8_t)*c >> 4]);
                    ss->append(1, hexdigits[*c & 0xf]);
                }
            } else if(ss) {
                ss->append(1, *c);
            }
        }
        if(!ss) {
            return str;
        } else {
            std::string rt = *ss;
            delete ss;
            return rt;
        }
    }

    std::string urlDecode(const std::string &str, bool space_as_plus) {
        std::string* ss = nullptr;
        const char* end = str.c_str() + str.length();
        for(const char* c = str.c_str(); c < end; ++c) {
            if(*c == '+' && space_as_plus) {
                if(!ss) {
                    ss = new std::string;
                    ss->append(str.c_str(), c - str.c_str());
                }
                ss->append(1, ' ');
            } else if(*c == '%' && (c + 2) < end
                      && isxdigit(*(c + 1)) && isxdigit(*(c + 2))){
                if(!ss) {
                    ss = new std::string;
                    ss->append(str.c_str(), c - str.c_str());
                }
                ss->append(1, (char)(x_digit_chars[(unsigned char)*(c + 1)] << 4 | x_digit_chars[(unsigned char)*(c + 2)]));
                c += 2;
            } else if(ss) {
                ss->append(1, *c);
            }
        }
        if(!ss) {
            return str;
        } else {
            std::string rt = *ss;
            delete ss;
            return rt;
        }
    }

    std::string trim(const std::string &str, const std::string &delimit) {
        auto begin = str.find_first_not_of(delimit);
        if (begin == std::string::npos) {
            return "";
        }
        auto end = str.find_last_not_of(delimit);
        return str.substr(begin, end - begin + 1);
    }

    std::string base64encode(const void* data, size_t len) {
        const char* base64 =
                "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

        std::string ret;
        ret.reserve(len * 4 / 3 + 2);

        const unsigned char* ptr = (const unsigned char*)data;
        const unsigned char* end = ptr + len;

        while(ptr < end) {
            unsigned int packed = 0;
            int i = 0;
            int padding = 0;
            for(; i < 3 && ptr < end; ++i, ++ptr) {
                packed = (packed << 8) | *ptr;
            }
            if(i == 2) {
                padding = 1;
            } else if (i == 1) {
                padding = 2;
            }
            for(; i < 3; ++i) {
                packed <<= 8;
            }

            ret.append(1, base64[packed >> 18]);
            ret.append(1, base64[(packed >> 12) & 0x3f]);
            if(padding != 2) {
                ret.append(1, base64[(packed >> 6) & 0x3f]);
            }
            if(padding == 0) {
                ret.append(1, base64[packed & 0x3f]);
            }
            ret.append(padding, '=');
        }

        return ret;
    }

    std::string sha1sum(const void *data, size_t len) {
        SHA_CTX ctx;
        SHA1_Init(&ctx);
        SHA1_Update(&ctx, data, len);
        std::string result;
        result.resize(SHA_DIGEST_LENGTH);
        SHA1_Final((unsigned char*)&result[0], &ctx);
        return result;
    }
}
