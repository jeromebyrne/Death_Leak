#pragma once
#include <algorithm>
#include <chrono>
#include <cmath>
#include <limits>

namespace deathleak::linux_port {
// One authoritative 60-Hz simulation clock. Presentation may omit intermediate
// states when rendering is slower; it must never alter the simulation delta.
class FixedStepPacer {
public:
    using Clock = std::chrono::steady_clock;
    static constexpr double StepSeconds = 1.0 / 60.0;
    static constexpr int MaxCatchUpSteps = 120;
    explicit FixedStepPacer(Clock::time_point start) : last_(start), pending_(1) {}
    void Accumulate(Clock::time_point now) {
        // Ignore a reversed clock sample. A large suspension must neither
        // overflow the integer step count nor create an unbounded update queue.
        // Dropped quanta remain observable through DroppedSteps().
        if (now <= last_) return;
        const double elapsed = std::chrono::duration<double>(now - last_).count();
        last_ = now;
        const double total = remainder_ + elapsed;
        const double steps = std::floor((total + 1e-7) / StepSeconds);
        remainder_ = std::clamp(total - steps * StepSeconds, 0.0, StepSeconds);
        const double room = static_cast<double>(MaxCatchUpSteps - pending_);
        const int admitted = static_cast<int>(std::min(steps, room));
        pending_ += admitted;
        // Extremely long suspensions are reported, not cast to int. Normal
        // 30..240-Hz cadence never reaches this path.
        const double excess = steps - admitted;
        if (excess > 0) {
            const auto capacity = std::numeric_limits<unsigned long long>::max() - dropped_;
            if (excess >= static_cast<double>(capacity))
                dropped_ = std::numeric_limits<unsigned long long>::max();
            else
                dropped_ += static_cast<unsigned long long>(excess);
        }
    }
    // Live startup can spend a long time compiling shaders/uploading assets
    // after its first update. Start elapsed gameplay time at the first present
    // so that one-time loading cannot create simulation debt or dropped steps.
    void RebaseAfterFirstPresent(Clock::time_point now) {
        last_ = now;
        remainder_ = 0.0;
        pending_ = 0;
    }
    bool Due() const { return pending_ > 0; }
    void Consume() { --pending_; }
    bool PresentNow() const { return pending_ == 0; }
    // Fraction of the next fixed step elapsed after all due updates are consumed.
    // Read-only: display-only slots must not advance gameplay or shader clocks.
    double PresentationAlpha() const {
        return pending_ ? 0.0 : std::clamp(remainder_ / StepSeconds, 0.0, 1.0);
    }
    double UntilNextSeconds() const { return std::max(0.0, StepSeconds - remainder_); }
    unsigned long long DroppedSteps() const { return dropped_; }
private:
    Clock::time_point last_;
    double remainder_ = 0;
    int pending_ = 0;
    unsigned long long dropped_ = 0;
};
}
