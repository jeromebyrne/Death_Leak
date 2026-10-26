#pragma once
#include <chrono>
#include <algorithm>
#include <cmath>

namespace deathleak::linux_port {
// Display slots are independent of the fixed simulation accumulator. Missing a
// slot skips it; it never asks the game to simulate or render twice to catch up.
class RefreshSlotScheduler {
public:
    using Clock = std::chrono::steady_clock;
    RefreshSlotScheduler(Clock::time_point origin, int hz) : origin_(origin), hz_(hz) {}
    bool Due(Clock::time_point now) const { return now >= Next(); }
    double UntilNextSeconds(Clock::time_point now) const {
        return std::max(0.0, std::chrono::duration<double>(Next() - now).count());
    }
    void Presented(Clock::time_point now) {
        const double elapsed = std::max(0.0, std::chrono::duration<double>(now - origin_).count());
        nextSlot_ = std::max(nextSlot_ + 1, static_cast<long long>(std::floor(elapsed * hz_)) + 1);
    }
    long long NextSlot() const { return nextSlot_; }
private:
    Clock::time_point Next() const {
        return origin_ + std::chrono::duration_cast<Clock::duration>(
            std::chrono::duration<double>(static_cast<double>(nextSlot_) / hz_));
    }
    Clock::time_point origin_;
    int hz_;
    long long nextSlot_ = 0;
};
}
