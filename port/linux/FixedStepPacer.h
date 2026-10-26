#pragma once
#include <algorithm>
#include <chrono>
#include <cmath>

namespace deathleak::linux_port {
// One authoritative 60-Hz simulation clock. Presentation may omit intermediate
// states when rendering is slower; it must never alter the simulation delta.
class FixedStepPacer {
public:
    using Clock = std::chrono::steady_clock;
    static constexpr double StepSeconds = 1.0 / 60.0;
    explicit FixedStepPacer(Clock::time_point start) : last_(start), pending_(1) {}
    void Accumulate(Clock::time_point now) {
        const double elapsed = std::chrono::duration<double>(now - last_).count();
        last_ = now;
        if (elapsed > 0.0) remainder_ += elapsed;
        const int due = static_cast<int>(std::floor((remainder_ + 1e-7) / StepSeconds));
        remainder_ -= due * StepSeconds;
        pending_ += due;
        // An overloaded machine cannot recover an unbounded queue. Reported
        // debt is intentionally retained up to two seconds of fixed updates.
        if (pending_ > 120) { dropped_ += pending_ - 120; pending_ = 120; }
    }
    bool Due() const { return pending_ > 0; }
    void Consume() { --pending_; }
    bool PresentNow() const { return pending_ == 0; }
    double UntilNextSeconds() const { return std::max(0.0, StepSeconds - remainder_); }
    unsigned long long DroppedSteps() const { return dropped_; }
private:
    Clock::time_point last_;
    double remainder_ = 0;
    int pending_ = 0;
    unsigned long long dropped_ = 0;
};
}
