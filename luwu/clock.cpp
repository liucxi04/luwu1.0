//
// Created by liucxi on 2022/11/9.
//

#include "clock.h"
#include "utils/util.h"

namespace luwu {

    bool Clock::Comparator::operator() (const Clock::ptr &lhs, const Clock::ptr &rhs) const {
        if (!lhs && !rhs) {
            return false;
        }
        if (!lhs) {
            return true;
        }
        if (!rhs) {
            return false;
        }
        if (lhs->time_ < rhs->time_) {
            return true;
        }
        if (lhs->time_ > rhs->time_) {
            return false;
        }
        return lhs.get() < rhs.get();
    }

    bool Clock::cancel() {
        RWMutex::WriteLock lock(manager_->mutex_);
        if (clock_callback_) {
            clock_callback_ = nullptr;
            auto it = manager_->clocks_.find(shared_from_this());
            manager_->clocks_.erase(it);
            return true;
        }
        return false;
    }

    bool Clock::refresh() {
        RWMutex::WriteLock lock(manager_->mutex_);
        if (clock_callback_) {
            auto it = manager_->clocks_.find(shared_from_this());
            if (it == manager_->clocks_.end()) {
                return false;
            }
            manager_->clocks_.erase(it);

            time_ = getCurrentTime() + period_;
            manager_->clocks_.insert(shared_from_this());
            return true;
        }
        return false;
    }

    bool Clock::reset(uint64_t period, bool from_now) {
        RWMutex::WriteLock lock(manager_->mutex_);
        if (clock_callback_) {
            auto it = manager_->clocks_.find(shared_from_this());
            if (it == manager_->clocks_.end()) {
                return false;
            }
            manager_->clocks_.erase(it);

            period_ = period;
            time_ = from_now ? getCurrentTime() + period_ : time_;
            // 在这里因为具体执行时间可能不变，所以插入时可能会在堆顶，可能会触发 onClockInsertAtFront，需要通过管理器来添加该定时器
            manager_->addClock(shared_from_this(), lock);
            return true;
        }
        return false;
    }

    // region # Clock::Clock()
    Clock::Clock(uint64_t time)
        : recurring_(false), period_(0), time_(time), clock_callback_(nullptr), manager_(nullptr) {
    }

    Clock::Clock(bool recurring, uint64_t period, std::function<void()> callback, ClockManager *manager)
        : recurring_(recurring), period_(period), time_(getCurrentTime() + period_)
        , clock_callback_(std::move(callback)), manager_(manager) {
    }
    // endregion

    Clock::ptr ClockManager::addClock(uint64_t period, std::function<void()> callback, bool recurring) {
        Clock::ptr clock1(new Clock(recurring, period, std::move(callback), this));
        RWMutex::WriteLock lock(mutex_);
        addClock(clock1, lock);
        return clock1;
    }

    Clock::ptr ClockManager::addCondClock(uint64_t period, const std::function<void()>& callback,
                                          const std::weak_ptr<void> &weak_cond, bool recurring) {
        return addClock(period, [weak_cond, callback]() {
            std::shared_ptr<void> tmp = weak_cond.lock();
            if (tmp) {
                callback();
            }
        }, recurring);
    }

    uint64_t ClockManager::getNextTime() {
        RWMutex::ReadLock lock(mutex_);
        tickled_ = false;
        if (clocks_.empty()) {
            return ~0ull;
        }

        const auto clock1 = *clocks_.begin();
        uint64_t now = getCurrentTime();
        if (now >= clock1->time_) {
            return 0;
        } else {
            return clock1->time_ - now;
        }
    }

    void ClockManager::listExpiredCallback(std::vector<std::function<void()>> &callbacks) {
        {
            RWMutex::ReadLock lock(mutex_);
            if (clocks_.empty()) {
                return;
            }
        }

        uint64_t now = getCurrentTime();
        std::vector<Clock::ptr> expired;
        RWMutex::WriteLock lock(mutex_);
        Clock::ptr clock1(new Clock(now));
        auto it = clocks_.upper_bound(clock1);
        expired.insert(expired.begin(), clocks_.begin(), it);

        for (auto &clock : expired) {
            callbacks.push_back(clock->clock_callback_);
            clocks_.erase(clock);
            if (clock->recurring_) {
                clock->time_ = now + clock->time_;
                clocks_.insert(clock);
            }
        }
    }

    void ClockManager::addClock(const Clock::ptr &clock, RWMutex::WriteLock &lock) {
        auto it = clocks_.insert(clock).first;
        if (it == clocks_.begin() && !tickled_) {
            tickled_ = true;
        }
        lock.unlock();
        if (tickled_) {
            onClockInsertAtFront();
        }
    }
}
