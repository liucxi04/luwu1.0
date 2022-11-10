//
// Created by liucxi on 2022/11/9.
//

#ifndef LUWU_CLOCK_H
#define LUWU_CLOCK_H

#include <set>
#include <memory>
#include <vector>
#include <functional>
#include "utils/mutex.h"

namespace luwu {
    class ClockManager;

    /**
     * @brief 定时器
     */
    class Clock : std::enable_shared_from_this<Clock> {
        friend class ClockManager;

    public:
        using ptr = std::shared_ptr<Clock>;

        /**
         * @brief 定时器回调函数
         */
        using clock_callback = std::function<void()>;

        /**
         * @brief 取消定时器
         * @return 操作是否成功
         */
        bool cancel();

        /**
         * @brief 重新设置定时器的执行时间
         * @details time_ = getCurrentTime() + period
         * @return 操作是否成功
         */
        bool refresh();

        /**
         * @brief 重置定时器
         * @param period 新的周期
         * @param from_now 是否重当前时间开始计时
         * @return 操作是否成功
         */
        bool reset(uint64_t period, bool from_now);

    private:
        /**
         * @brief 私有构造函数，用在 ClockManager::clocks_ 的二分查找中
         * @param time 定时器发生的时间
         */
        explicit Clock(uint64_t time);

        /**
         * @brief 私有构造函数
         * @param recurring 是否重复
         * @param period 周期
         * @param callback 定时器回调函数
         * @param manager 所属的定时器管理器
         */
        Clock(bool recurring, uint64_t period, clock_callback callback, ClockManager *manager);

    private:
        /// 是否重复
        bool recurring_;
        /// 执行周期
        uint64_t period_;
        /// 具体的执行时间
        uint64_t time_;
        /// 定时器回调函数
        clock_callback clock_callback_;
        /// 定时器所属的管理器
        ClockManager *manager_;

    private:
        struct Comparator {
            bool operator() (const Clock::ptr &lhs, const Clock::ptr &rhs) const;
        };
    };

    /**
     * @brief 定时器管理器
     */
    class ClockManager {
        friend class Clock;

    public:
        /**
         * @brief 构造函数
         */
        ClockManager() : tickled_(false) {}

        /**
         * @brief 默认虚析构函数
         */
        virtual ~ClockManager() = default;

        /**
         * @brief 向管理器新增一个定时器
         * @param period 周期
         * @param callback 定时器回调函数
         * @param recurring 是否重复
         * @return 新增的定时器智能指针
         */
        Clock::ptr addClock(uint64_t period, Clock::clock_callback callback, bool recurring = false);

        /**
         * @brief 向管理器新增一个条件定时器
         * @param period 周期
         * @param callback 定时器回调函数
         * @param weak_cond 弱智能指针作为条件
         * @param recurring 是否重复
         * @return 新增的定时器智能指针
         */
        Clock::ptr addCondClock(uint64_t period, const Clock::clock_callback& callback,
                                const std::weak_ptr<void> &weak_cond, bool recurring = false);

        /**
         * @brief 获得距离最近发生的定时器的时间
         * @return 距离最近发生的定时器的时间
         */
        uint64_t getNextTime();

        /**
         * @brief 列出所有超时的定时器需要执行的回调函数
         * @param callbacks 所有需要执行的回调函数
         */
        void listExpiredCallback(std::vector<Clock::clock_callback> &callbacks);

    private:
        /**
         * @brief 当插入一个定时器到堆顶时需要执行的操作
         */
        virtual void onClockInsertAtFront() {};

        /**
         * @brief 向管理器新增一个定时器，有锁
         * @param clock 新增的定时器
         * @param lock 写锁
         */
       void addClock(const Clock::ptr &clock, RWMutex::WriteLock &lock);

    private:
        RWMutex mutex_;
        /// 管理的所有定时器，按发生时间排列
        std::set<Clock::ptr, Clock::Comparator> clocks_;
        /// 是否需要通知
        /// TODO ???
        bool tickled_;
    };
}
#endif //LUWU_CLOCK_H
