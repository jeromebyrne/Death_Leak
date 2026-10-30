#pragma once
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <vector>

namespace deathleak::game {
// Source: Game/src/GameObjects/{Platform,PathingPlatform,FallingPlatform,
// MovingSprite,GameObject}.cpp and Game/src/SineWave.cpp. No renderer/XML deps.
// World Y is up. Velocity/speed/maxVelocity are ORIGINAL units per 60-Hz tick,
// not units/second. Update once per eligible fixed simulation tick. now is the
// caller's source-equivalent total gameplay clock (not wall clock).
class AuthoredPlatformMotion {
public:
    struct Point { float x = 0, y = 0; };
    enum class Kind { Static, Pathing, Falling };
    enum class PathState { NotPathing, Forward, Backward, Returning };
    enum class FallState { Static, Triggered, Falling, Returning };
    struct Config {
        Kind kind = Kind::Static;
        Point origin, speed{1, 1}, maxVelocity{0, 0};
        float resistanceX = 1, resistanceY = 1, gravity = 1;
        bool applyGravity = false;
        bool pathWhenTriggered = false, pathForward = true;
        // ABSOLUTE world points: add XML point offsets to authored origin first,
        // then apply level scaling to positions AND speed/maxVelocity.
        std::vector<Point> points;
        float maxTriggerTime = 1.5f, timeUntilReturn = 5.f, alpha = 1;
        bool sineActive = false;
        float sineOffsetX = 0, sineOffsetY = 0, sineSpeed = 0, sineInitialStep = 0;
    };

    AuthoredPlatformMotion() = default;
    explicit AuthoredPlatformMotion(const Config& config) { Reset(config); }
    void Reset(const Config& config) {
        config_ = config;
        // MovingSprite::XmlRead treats negative max Y as unlimited (99999).
        if (config_.maxVelocity.y < 0) config_.maxVelocity.y = 99999.f;
        position_ = previous_ = config_.origin;
        velocity_ = {}; accelerationY_ = 0;
        alpha_ = config_.alpha; phase_ = config_.sineInitialStep;
        sineActive_ = config_.sineActive;
        gravityActive_ = config_.kind == Kind::Falling ? false : config_.applyGravity;
        pathState_ = config_.kind == Kind::Pathing && !config_.pathWhenTriggered
            ? (config_.pathForward ? PathState::Forward : PathState::Backward)
            : PathState::NotPathing;
        fallState_ = FallState::Static; index_ = 0;
        closestSquared_ = std::numeric_limits<float>::infinity();
        triggerStart_ = fallStart_ = returnStart_ = 0; returnPosition_ = {};
    }
    const Config& Configuration() const { return config_; }
    Point Position() const { return position_; }
    Point PreviousPosition() const { return previous_; }
    Point Delta() const { return {position_.x - previous_.x, position_.y - previous_.y}; }
    Point Velocity() const { return velocity_; }
    float Alpha() const { return alpha_; }
    bool SineActive() const { return sineActive_; }
    PathState PathPhase() const { return pathState_; }
    FallState FallPhase() const { return fallState_; }
    std::size_t TargetIndex() const { return index_; }

    // Call ONLY for an actual broad/narrow-phase collision. Original trigger
    // uses drawable Left/Right and Y, NOT collision-box offsets/dimensions.
    static bool PlayerCanTrigger(float playerX, float playerBottom, float playerVy,
                                 float left, float right, float platformY) {
        return playerBottom > platformY && playerX > left && playerX < right && playerVy <= 0;
    }
    // qualifyingTopContact uses PlayerCanTrigger above. Pathing's contact also
    // assigns the player's objectMovingWith; caller owns that stable identity.
    void OnPlayerCollision(bool qualifyingTopContact, double now) {
        if (!qualifyingTopContact) return;
        if (config_.kind == Kind::Falling && fallState_ == FallState::Static) {
            sineActive_ = false; TriggerFall(now);
        }
        if (config_.kind != Kind::Pathing || !config_.pathWhenTriggered ||
            (pathState_ != PathState::NotPathing && pathState_ != PathState::Returning)) return;
        closestSquared_ = std::numeric_limits<float>::infinity();
        if (pathState_ == PathState::Returning && !config_.points.empty()) {
            if (config_.pathForward) index_ = index_ + 1 < config_.points.size() ? index_ + 1 : 0;
            else if (index_ > 0) --index_;
        }
        pathState_ = config_.pathForward ? PathState::Forward : PathState::Backward;
    }
    // OnDamage(..., shouldExplode): unlike player contact, source does NOT
    // disable sine. Caller checks can_be_damaged before delivering damage.
    void OnExplosion(double now) {
        if (config_.kind == Kind::Falling && fallState_ == FallState::Static) TriggerFall(now);
    }
    void Update(float dt, double now, bool playerMovingWith = false) {
        previous_ = position_;
        if (!(dt > 0)) return;
        // Falling state logic precedes base physics; path logic follows it.
        if (config_.kind == Kind::Falling) UpdateFall(now);
        if (sineActive_) {
            phase_ += config_.sineSpeed * dt;
            position_ = {config_.origin.x + config_.sineOffsetX * std::sin(phase_),
                         config_.origin.y + config_.sineOffsetY * std::sin(phase_)};
        }
        const float ticks = dt * 60.f;
        velocity_.x = Clamp(velocity_.x, config_.maxVelocity.x);
        velocity_.y = Clamp(velocity_.y + accelerationY_, config_.maxVelocity.y);
        // Gravity writes NEXT tick's acceleration, not this tick's velocity.
        if (gravityActive_) {
            const float multiplier = velocity_.y > 0 || velocity_.y < -15.f ? 1.f : 3.f;
            accelerationY_ = -config_.gravity * multiplier / config_.resistanceY * ticks;
        }
        position_.x += velocity_.x * ticks; position_.y += velocity_.y * ticks;
        velocity_.x *= config_.resistanceX;
        if (std::abs(velocity_.x) < .1f) velocity_.x = 0;
        if (config_.kind == Kind::Pathing) UpdatePath(playerMovingWith);
    }
private:
    static float Clamp(float value, float limit) { return std::max(-limit, std::min(limit, value)); }
    void TriggerFall(double now) { fallState_ = FallState::Triggered; triggerStart_ = now; }
    void UpdateFall(double now) {
        switch (fallState_) {
        case FallState::Static: gravityActive_ = false; break;
        case FallState::Triggered: {
            const double fallAt = triggerStart_ + config_.maxTriggerTime;
            if (fallAt < now) {
                alpha_ = 1; fallState_ = FallState::Falling; fallStart_ = now; gravityActive_ = true;
                break;
            }
            // Source's four linear quarters: 1 -> .5 -> 1 -> .5 -> 1.
            const float p = config_.maxTriggerTime > 0
                ? static_cast<float>((fallAt - now) / config_.maxTriggerTime) : 0;
            if (p > .75f) alpha_ = .5f + ((p * 4 - 3) * .5f);
            else if (p > .5f) alpha_ = .5f + ((3 - p * 4) * .5f);
            else if (p > .25f) alpha_ = .5f + ((p * 4 - 1) * .5f);
            else alpha_ = .5f + ((1 - p * 4) * .5f);
            break;
        }
        case FallState::Falling:
            if (fallStart_ + config_.timeUntilReturn < now) {
                returnStart_ = now; returnPosition_ = position_;
                fallState_ = FallState::Returning; gravityActive_ = false;
                velocity_ = {}; accelerationY_ = 0;
            }
            break;
        case FallState::Returning: {
            const double left = returnStart_ + 2.0 - now;
            if (left < .01) { fallState_ = FallState::Static; position_ = config_.origin; }
            else {
                const float t = static_cast<float>(1 - left / 2.0);
                position_ = {returnPosition_.x + (config_.origin.x - returnPosition_.x) * t,
                             returnPosition_.y + (config_.origin.y - returnPosition_.y) * t};
            }
            break;
        }
        }
    }
    void UpdatePath(bool riding) {
        if (pathState_ == PathState::NotPathing) { velocity_ = {}; return; }
        if (pathState_ != PathState::Returning && config_.pathWhenTriggered && !riding) {
            pathState_ = PathState::Returning;
            closestSquared_ = std::numeric_limits<float>::infinity();
            if (config_.pathForward) { if (index_ > 0) --index_; }
            else index_ = index_ + 1 >= config_.points.size() ? 0 : index_ + 1;
            return; // Source retains previous velocity on transition ticks.
        }
        if (config_.points.empty() || index_ >= config_.points.size()) return;
        Point d{config_.points[index_].x - position_.x, config_.points[index_].y - position_.y};
        const float squared = d.x * d.x + d.y * d.y;
        // Strict <10 radius OR increasing distance (overshoot), no snapping.
        if (squared < 100.f || squared > closestSquared_) {
            closestSquared_ = std::numeric_limits<float>::infinity();
            if (pathState_ == PathState::Returning && index_ == 0) {
                pathState_ = PathState::NotPathing; return;
            }
            const bool forward = pathState_ == PathState::Forward ||
                (pathState_ == PathState::Returning && !config_.pathForward);
            if (forward) index_ = index_ + 1 < config_.points.size() ? index_ + 1 : 0;
            else index_ = index_ > 0 ? index_ - 1 : config_.points.size() - 1;
            return;
        }
        closestSquared_ = squared;
        const float length = std::sqrt(squared);
        velocity_ = {Clamp(d.x / length * config_.speed.x, config_.maxVelocity.x),
                     Clamp(d.y / length * config_.speed.y, config_.maxVelocity.y)};
    }
    Config config_;
    Point position_, previous_, velocity_, returnPosition_;
    float accelerationY_ = 0, alpha_ = 1, phase_ = 0;
    bool sineActive_ = false, gravityActive_ = false;
    PathState pathState_ = PathState::NotPathing;
    FallState fallState_ = FallState::Static;
    std::size_t index_ = 0;
    float closestSquared_ = std::numeric_limits<float>::infinity();
    double triggerStart_ = 0, fallStart_ = 0, returnStart_ = 0;
};
}
