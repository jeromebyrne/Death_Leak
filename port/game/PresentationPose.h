#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace deathleak::game {
// A fixed-update-owned, immutable-to-presentation pair of world-space samples.
// The caller decides whether consecutive samples belong to the same scene and
// continuous trajectory. No gameplay state, clock, input, RNG or resource is read
// when sampling a display slot.
struct PresentationPose {
    float playerX = 0.f;
    float playerY = 0.f;
    float cameraX = 0.f;
    float cameraY = 0.f;
    std::uint64_t sceneGeneration = 0;
};

class PresentationPoseHistory {
public:
    void Seal(const PresentationPose& pose, bool continuous = true) {
        previous_ = current_;
        current_ = pose;
        validPair_ = hasCurrent_ && continuous &&
            previous_.sceneGeneration == current_.sceneGeneration &&
            std::isfinite(previous_.playerX) && std::isfinite(previous_.playerY) &&
            std::isfinite(previous_.cameraX) && std::isfinite(previous_.cameraY) &&
            std::isfinite(current_.playerX) && std::isfinite(current_.playerY) &&
            std::isfinite(current_.cameraX) && std::isfinite(current_.cameraY) &&
            std::fabs(current_.playerX - previous_.playerX) <= 200.f &&
            std::fabs(current_.playerY - previous_.playerY) <= 200.f &&
            std::fabs(current_.cameraX - previous_.cameraX) <= 200.f &&
            std::fabs(current_.cameraY - previous_.cameraY) <= 200.f;
        hasCurrent_ = true;
    }
    PresentationPose Sample(double alpha) const {
        if (!validPair_ || !std::isfinite(alpha) || alpha >= 1.0) return current_;
        const float t = static_cast<float>(std::clamp(alpha, 0.0, 1.0));
        auto blend = [t](float a, float b) { return a + (b - a) * t; };
        return {blend(previous_.playerX, current_.playerX),
                blend(previous_.playerY, current_.playerY),
                blend(previous_.cameraX, current_.cameraX),
                blend(previous_.cameraY, current_.cameraY),
                current_.sceneGeneration};
    }
    bool HasContinuousPair() const { return validPair_; }
private:
    PresentationPose previous_{};
    PresentationPose current_{};
    bool hasCurrent_ = false;
    bool validPair_ = false;
};
}
