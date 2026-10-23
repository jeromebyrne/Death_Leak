#pragma once

namespace deathleak::game {
// Camera2D::Update snapshots the view *before* modifying the stored position.
// Caller follows/clamps first, starts shake after player processing, then samples
// here once per rendered update. Signs are injectable to isolate source rand().
class CameraShake {
public:
    struct Sample {
        float viewX, viewY, storedX, storedY, remaining;
        bool active;
    };
    void Reset() { active_ = false; start_ = 0.f; }
    bool Start(float intensity, float duration, float totalTime) {
        if (active_) return false;
        intensity_ = intensity;
        duration_ = duration;
        active_ = true;
        start_ = totalTime;
        return true;
    }
    bool StartMedium(float totalTime) { return Start(15.f, 0.30f, totalTime); }
    Sample Update(float totalTime, float timeModifier, bool followX, bool followY,
                  int signX, int signY, float& cameraX, float& cameraY) {
        const float viewX = cameraX, viewY = cameraY;
        float remaining = 0.f;
        if (active_) {
            remaining = start_ + duration_ - totalTime;
            if (remaining <= 0.f) { active_ = false; remaining = 0.f; }
            else {
                const float amount = intensity_ * (remaining / duration_) * timeModifier;
                if (followX) cameraX += (signX < 0 ? -amount : amount);
                if (followY) cameraY += (signY < 0 ? -amount : amount);
            }
        }
        return {viewX, viewY, cameraX, cameraY, remaining, active_};
    }
    bool Active() const { return active_; }
    bool WillJitter(float totalTime) const { return active_ && totalTime < start_ + duration_; }
private:
    bool active_ = false;
    float start_ = 0.f;
    float intensity_ = 15.f;
    float duration_ = 0.30f;
};
}
