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
constexpr float kWallJumpSpeedX = 15.f * 60.f;
constexpr float kWallJumpSpeedY = 20.f * 60.f;
}
void PlayerMotion::Reset(float x, float y) {
    x_ = x; y_ = y; vx_ = vy_ = 0; sinceGround_ = 1; jumpHeld_ = false; grounded_ = false;
    wallSide_ = 0; wallJumpRemaining_ = 0; jumpPercent_ = 40.f;
}
void PlayerMotion::Update(const LevelScene& scene, float dt, int direction, bool jump) {
    if (dt <= 0) return;
    const float ticks = dt * 60.f;
    bool inWater = false;
    for (const auto& hazard : scene.Hazards())
        if (hazard.water && std::abs(x_-hazard.x) <= hazard.width*0.5f &&
            std::abs(y_-hazard.y) <= hazard.height*0.5f) inWater = true;
    if (wallJumpRemaining_ > 0) wallJumpRemaining_ = std::max(0.f, wallJumpRemaining_ - dt);
    if (wallJumpRemaining_ <= 0) {
        if (direction) vx_ = std::clamp(vx_ + direction * 60.f * ticks, -kMaxSpeed*(inWater ? 0.4f : 1.f), kMaxSpeed*(inWater ? 0.4f : 1.f));
        else vx_ *= std::pow(0.80f, ticks); // Player::UpdateResistance without acceleration
    } else vx_ *= std::pow(0.99f, ticks);
    if (std::abs(vx_) < 6.f) vx_ = 0;
    // Character::CanJump permits a 0.3 s post-edge window.
    if (inWater && jump) {
        // Gamepad swim applies AccelerateY(1, 0.3) each target update.
        // MovingSprite clamps submerged vertical speed to four units/tick.
        vy_ = std::min(4.f*60.f, vy_ + 0.3f*60.f*ticks);
    } else if (jump && !jumpHeld_ && (grounded_ || sinceGround_ <= 0.3f || wallSide_)) {
        if (!grounded_ && sinceGround_ > 0.3f && wallSide_) {
            vx_ = -wallSide_ * kWallJumpSpeedX;
            vy_ = kWallJumpSpeedY;
            wallJumpRemaining_ = 0.3f;
        } else { vy_ = kJumpSpeed * 0.4f; jumpPercent_ = 40.f; }
        y_ += 10.f; // Character::Jump releases the solid line before ascent.
        grounded_ = false; sinceGround_ = 1.f;
    }
    // InputManager::ProcessJump_* starts at 40% and increases by 6.5%
    // on each subsequent held update, up to the authored max jump speed.
    // Character::IncreaseJump only raises velocity, never lowers it.
    if (jump && jumpHeld_ && !inWater && !grounded_ && sinceGround_ > 0.3f && wallJumpRemaining_ <= 0 && jumpPercent_ < 100.f) {
        jumpPercent_ = std::min(100.f, jumpPercent_ + 6.5f * ticks);
        vy_ = std::max(vy_, kJumpSpeed * jumpPercent_ / 100.f);
    }
    if (!jump) jumpPercent_ = 40.f;
    jumpHeld_ = jump;
    const float oldX = x_, oldY = y_;
    const float targetX = x_ + vx_ * dt;
    x_ = targetX;
    wallSide_ = 0;
    // Near-vertical authored line segments act as side walls. Sweep the player
    // collision box, so a high-speed frame cannot step through one.
    for (const auto& line : scene.Ground()) {
        if (std::abs(line.x2-line.x1) > 1.f) continue;
        const float wallX = (line.x1+line.x2)*0.5f;
        const float low = std::min(line.y1,line.y2), high = std::max(line.y1,line.y2);
        if (oldY+kHalfHeight < low || oldY-kHalfHeight > high) continue;
        if (vx_ > 0 && oldX+kHalfWidth <= wallX && x_+kHalfWidth > wallX)
            { x_ = wallX-kHalfWidth; vx_ = 0; wallSide_ = 1; }
        else if (vx_ < 0 && oldX-kHalfWidth >= wallX && x_-kHalfWidth < wallX)
            { x_ = wallX+kHalfWidth; vx_ = 0; wallSide_ = -1; }
    }
    for (const auto& box : scene.SolidBoxes()) {
        if (oldY+kHalfHeight <= box.bottom || oldY-kHalfHeight >= box.top) continue;
        if (vx_ > 0 && oldX+kHalfWidth <= box.left && x_+kHalfWidth > box.left)
            { x_ = box.left-kHalfWidth; vx_ = 0; wallSide_ = 1; }
        else if (vx_ < 0 && oldX-kHalfWidth >= box.right && x_-kHalfWidth < box.right)
            { x_ = box.right+kHalfWidth; vx_ = 0; wallSide_ = -1; }
    }
    if (!grounded_) {
        // Original MovingSprite applies gravity after velocity integration;
        // this continuous equivalent preserves the source jump/gravity scale.
        const float wallGravity = wallSide_ ? kGravity * 0.5f : kGravity;
        const float gravity = inWater ? kGravity * 0.1f :
            (vy_ < 0 && vy_ > -15.f*60.f ? wallGravity*3.f : wallGravity);
        y_ += vy_*dt - 0.5f*gravity*dt*dt;
        vy_ = std::max(inWater ? -4.f*60.f : -34.f*60.f, vy_-gravity*dt);
        if (wallSide_ && vy_ < 0) vy_ *= std::pow(0.7f, ticks);
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
    for (const auto& box : scene.SolidBoxes()) {
        if (x_+kHalfWidth <= box.left || x_-kHalfWidth >= box.right) continue;
        if ((wasGrounded && std::abs(box.top-(oldY-kHalfHeight)) <= 20.f) ||
            (vy_ <= 0 && oldY-kHalfHeight >= box.top-4.f && y_-kHalfHeight <= box.top))
            best = std::max(best, box.top);
        // Reject upward penetration through the underside of a solid object.
        if (vy_ > 0 && oldY+kHalfHeight <= box.bottom && y_+kHalfHeight > box.bottom) {
            y_ = box.bottom-kHalfHeight;
            vy_ = 0;
        }
    }
    if (best > -1e8f) { y_ = best+kHalfHeight; vy_ = 0; grounded_ = true; sinceGround_ = 0; }
    else sinceGround_ += dt;
}
}
