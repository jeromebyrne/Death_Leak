#pragma once

namespace deathleak::game {
// Process-owned shader timing. Total time advances with each main-loop update,
// including menus and modals. Pixel wobble advances at draw using the preceding
// rendered update's modified delta, matching Game::Draw/program.cpp ordering.
struct ShaderClockSnapshot {
    float totalTime = 0.f;
    float pixelWobbleTime = 0.f;
};

class ShaderClock {
public:
    void Update(float modifiedDelta) { totalTime_ += modifiedDelta; }
    ShaderClockSnapshot Draw() {
        pixelWobbleTime_ += reversing_ ? -previousRenderDelta_ : previousRenderDelta_;
        if (!reversing_ && pixelWobbleTime_ > 10.f) reversing_ = true;
        else if (reversing_ && pixelWobbleTime_ < 5.f) reversing_ = false;
        return {totalTime_, pixelWobbleTime_};
    }
    void RenderCompleted(float modifiedDelta) { previousRenderDelta_ = modifiedDelta; }
    bool Reversing() const { return reversing_; }
private:
    float totalTime_ = 0.f;
    float pixelWobbleTime_ = 0.f;
    float previousRenderDelta_ = 0.f;
    bool reversing_ = false;
};
}
