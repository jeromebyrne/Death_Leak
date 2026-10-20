#include "PlayerMotion.h"
#include <algorithm>
#include <cmath>

namespace deathleak::game {
namespace {
constexpr float kHalfWidth = 128.959793f / 2.f; // authored Player collision box
constexpr float kHalfHeight = 214.898743f / 2.f;
constexpr float kMaxSpeed = 8.f * 60.f;
constexpr float kJumpSpeed = 21.f * 60.f;
constexpr float kGravity = (0.75f / 0.55f) * 60.f * 60.f;
}
void PlayerMotion::Reset(float x, float y) {
    x_ = x; y_ = y; vx_ = vy_ = 0; sinceGround_ = 1; jumpHeld_ = false; grounded_ = false;
}
void PlayerMotion::Update(const LevelScene& scene, float dt, int direction, bool jump) {
    if (dt <= 0) return;
    const float ticks = dt * 60.f;
    if (direction) vx_ = std::clamp(vx_ + direction * 60.f * ticks, -kMaxSpeed, kMaxSpeed);
    else vx_ *= std::pow(0.80f, ticks); // Player::UpdateResistance without acceleration
    if (std::abs(vx_) < 6.f) vx_ = 0;
    // Character::CanJump permits a 0.3 s post-edge window.
    if (jump && !jumpHeld_ && (grounded_ || sinceGround_ <= 0.3f)) {
        vy_ = kJumpSpeed;
        y_ += 10.f; // Character::Jump releases the solid line before ascent.
        grounded_ = false; sinceGround_ = 1.f;
    }
    jumpHeld_ = jump;
    const float oldX = x_, oldY = y_;
    const float targetX = x_ + vx_ * dt;
    x_ = targetX;
    // Near-vertical authored line segments act as side walls. Sweep the player
    // collision box, so a high-speed frame cannot step through one.
    for (const auto& line : scene.Ground()) {
        if (std::abs(line.x2-line.x1) > 1.f) continue;
        const float wallX = (line.x1+line.x2)*0.5f;
        const float low = std::min(line.y1,line.y2), high = std::max(line.y1,line.y2);
        if (oldY+kHalfHeight < low || oldY-kHalfHeight > high) continue;
        if (vx_ > 0 && oldX+kHalfWidth <= wallX && x_+kHalfWidth > wallX)
            { x_ = wallX-kHalfWidth; vx_ = 0; }
        else if (vx_ < 0 && oldX-kHalfWidth >= wallX && x_-kHalfWidth < wallX)
            { x_ = wallX+kHalfWidth; vx_ = 0; }
    }
    if (!grounded_) {
        // Original MovingSprite applies gravity after velocity integration;
        // this continuous equivalent preserves the source jump/gravity scale.
        const float gravity = vy_ < 0 && vy_ > -15.f*60.f ? kGravity*3.f : kGravity;
        y_ += vy_*dt - 0.5f*gravity*dt*dt;
        vy_ = std::max(-34.f*60.f, vy_-gravity*dt);
    }
    const bool wasGrounded = grounded_;
    grounded_ = false;
    float best = -1e9f;
    for (const auto& line : scene.Ground()) {
        const float dx = line.x2-line.x1;
        if (std::abs(dx) <= 1.f) continue;
        const float t = (x_-line.x1)/dx;
        if (t < 0 || t > 1) continue;
        const float surface = line.y1+t*(line.y2-line.y1);
        if ((wasGrounded && std::abs(surface-(oldY-kHalfHeight)) <= 20.f) ||
            (vy_ <= 0 && oldY-kHalfHeight >= surface-4.f && y_-kHalfHeight <= surface))
            best = std::max(best,surface);
    }
    if (best > -1e8f) { y_ = best+kHalfHeight; vy_ = 0; grounded_ = true; sinceGround_ = 0; }
    else sinceGround_ += dt;
}
}
