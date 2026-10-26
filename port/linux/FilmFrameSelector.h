#pragma once
#include <cstddef>
#include <cstdint>

namespace deathleak::linux_port {
// The original widget starts with switch time zero. Strict integer 1000/11-ms
// threshold, one-frame advance, and reset-to-now are authored behavior.
class FilmFrameSelector {
public:
    void Prepare(std::uint64_t nowMilliseconds, std::size_t frameCount) {
        if (frameCount && lastSwitchMilliseconds_ + 1000 / 11 < nowMilliseconds) {
            index_ = (index_ + 1) % frameCount;
            lastSwitchMilliseconds_ = nowMilliseconds;
        }
    }
    std::size_t Index() const { return index_; }
private:
    std::uint64_t lastSwitchMilliseconds_ = 0;
    std::size_t index_ = 0;
};
}
