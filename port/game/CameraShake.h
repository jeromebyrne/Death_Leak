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
    bool StartMedium(float totalTime) {
        if (active_) return false;
        active_ = true; start_ = totalTime; return true;
    }
    Sample Update(float totalTime, float timeModifier, bool followX, bool followY,
                  int signX, int signY, float& cameraX, float& cameraY) {
        const float viewX = cameraX, viewY = cameraY;
        float remaining = 0.f;
        if (active_) {
            remaining = start_ + 0.30f - totalTime;
            if (remaining <= 0.f) { active_ = false; remaining = 0.f; }
            else {
                const float amount = 15.f * (remaining / 0.30f) * timeModifier;
                if (followX) cameraX += (signX < 0 ? -amount : amount);
                if (followY) cameraY += (signY < 0 ? -amount : amount);
            }
        }
        return {viewX, viewY, cameraX, cameraY, remaining, active_};
    }
    bool Active() const { return active_; }
    bool WillJitter(float totalTime) const { return active_ && totalTime < start_ + 0.30f; }
private:
    bool active_ = false;
    float start_ = 0.f;
};
}
