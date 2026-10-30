#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <limits>

namespace deathleak::game {
// Bounded, renderer/XML-independent port of Game/src/GameObjects/{AmbientBird,
// ActiveBird,Rabbit,butterfly,MovingSprite,Character,NPC}.cpp and
// Game/src/AI/AIState{GroundAnimalWander,ButterflyWander,Follow,Repel}.cpp.
// All positions, dimensions, velocities and distance gates use ORIGINAL world
// units (Y UP); velocity is units per 60-Hz tick, NOT units/sec. Caller parses
// XML into Config, and converts coordinates only at its scene boundary.
// Update exactly once per eligible fixed simulation tick. No catch-up loop.
//
// Deliberate boundaries: private deterministic RNG (not source global rand);
// no damage/death/debris/audio/water/overlap/side-wall collision solver, source
// sine-wave transform, slow motion, or NPC-manager/triggers. Caller supplies
// terrain, hard-edge contact and level/view gates; omitted terrain means free
// flight, NOT a fabricated flat floor. ActiveBird differs here only in flap
// timings: its source damage/debris is outside this motion helper.
class AuthoredAmbientActor {
public:
    struct Point { float x = 0, y = 0; };
    enum class Kind { AmbientBird, Rabbit, Butterfly, ActiveBird };
    enum class RabbitMode { Wander, Follow, Repel };
    enum class WanderState { Moving, Resting, RunningFromPlayer };
    struct Config {
        Kind kind = Kind::AmbientBird;
        Point origin{}, dimensions{}, maxVelocity{};
        // XML: resistance x/y, xaccelrate/value, maxjumpspeed/value,
        // applygravity/value, travel_offset/value, horizontalflip/value.
        float resistanceX = 1, resistanceY = 1, accelX = 0;
        float maxJumpSpeed = 10, gravity = 1, travelOffset = 1000;
        bool applyGravity = true, background = false, horizontalFlip = false;
        // background MUST mean source numeric depth_layer > kGround (4500),
        // including kGroundBack, kNear/Middle/FarBackground and kMoon.
        // Source Character collisionboxdimensions: bottom = y+offsetY-height/2.
        float collisionHeight = 32, collisionOffsetY = 0;
        // Animation XML sequence rates and running limits (rabbit_anim: 2/10).
        float stillRate = 2, runningMaxRate = 10, runningMinRate = 0;
        float jumpingUpRate = 4, jumpingDownRate = 4;
        std::uint32_t seed = 1;
    };
    struct Environment {
        bool hasPlayer = false;
        Point player{};
        bool playerOnGround = true;
        // Source world floor Y for actor collision-bottom, NaN if no surface.
        // Arguments are candidate X and PREVIOUS collision-bottom Y, permitting
        // caller to select a supporting floor rather than a ceiling/platform.
        std::function<float(float, float)> groundHeight;
        // Exact source recycle condition needs two DIFFERENT view queries.
        // objectInView is full drawable bounds at current pose; startInView
        // uses source start-position parallax offsets. Caller evaluates those.
        // Missing visibility data disables recycling (safe, no visible pop).
        bool visibilityKnown = false, objectInView = true, startInView = true;
        // Preferred: callbacks are evaluated AFTER motion, at source query time.
        // Capture source camera/parallax state and drawable bounds in closures.
        // Supplying both callbacks implies visibilityKnown automatically.
        std::function<bool(Point)> objectInViewAt, startInViewAt;
        bool hittingSolidLineEdge = false;
        float levelLeft = -std::numeric_limits<float>::infinity();
        float levelRight = std::numeric_limits<float>::infinity();
    };
    struct Pose {
        float x = 0, y = 0;
        int facing = 1; // +1 unflipped / -1 horizontally flipped
        const char* sequence = "Still";
        float rate = 2;
        bool loop = true, recycled = false;
    };

    AuthoredAmbientActor() = default;
    explicit AuthoredAmbientActor(const Config& config) { Reset(config); }
    void Reset(const Config& config) {
        c_ = config; rng_ = c_.seed ? c_.seed : 1;
        if (c_.maxVelocity.y < 0) c_.maxVelocity.y = 99999.f;
        pos_ = c_.origin; velocity_ = {}; ax_ = ay_ = 0;
        dirX_ = 1; facing_ = c_.horizontalFlip ? -1 : 1;
        resistanceX_ = currentResistanceX_ = c_.resistanceX;
        resistanceY_ = currentResistanceY_ = c_.resistanceY;
        maxX_ = c_.maxVelocity.x;
        gravityActive_ = c_.applyGravity;
        grounded_ = false; rising_ = falling_ = false; fallTime_ = airTime_ = 0;
        gliding_ = false; flapWait_ = flapEndWait_ = 0;
        yWait_ = lastBurst_ = directionWait_ = 0;
        mode_ = RabbitMode::Wander; wander_ = WanderState::Moving;
        moveTime_ = restTime_ = runTime_ = 0;
        stopDistance_ = 120;
        if (IsBird()) {
            gravityActive_ = false;
            dirX_ = c_.travelOffset < 0 ? -1.f : c_.travelOffset > 0 ? 1.f : 0.f;
        } else if (c_.kind == Kind::Butterfly) {
            // Butterfly::Initialise calls SetState/OnTransition BEFORE its
            // overrides: background OnTransition's 20.1 Y resistance is then
            // overwritten to 6.0, not preserved.
            gravityActive_ = true; resistanceX_ = .99f; resistanceY_ = 6;
            maxX_ = .5f; c_.maxVelocity.y = 5; c_.accelX = .5f;
        } else {
            stopDistance_ = 120.f + static_cast<float>(Random(100));
            BeginWander();
        }
        pose_ = {pos_.x, pos_.y, facing_, "Still", c_.stillRate, true, false};
    }
    const Config& Configuration() const { return c_; }
    Point Position() const { return pos_; }
    Point Velocity() const { return velocity_; }
    bool IsGliding() const { return gliding_; }
    bool IsGrounded() const { return grounded_; }
    WanderState WanderPhase() const { return wander_; }
    float StopDistance() const { return stopDistance_; }
    // Only explicit NPC-manager/trigger state changes should call this. Rabbit
    // initialises in Wander, NOT Follow. Repel ignores stopDistance in source.
    void SetRabbitMode(RabbitMode mode) {
        if (mode_ == mode) return;
        mode_ = mode;
        if (mode == RabbitMode::Wander) BeginWander();
    }
    Pose Update(float dt, const Environment& env) {
        if (!(dt > 0) || !std::isfinite(dt)) return pose_;
        // Sprite::Update calls UpdateAnimations BEFORE MovingSprite physics,
        // Character state updates, and NPC AI: preserve this one-tick latency.
        Animate();
        pose_.recycled = false;
        const float tickScale = dt * 60.f;
        const float oldBottom = Bottom();
        // First contact is resolved here only if a terrain callback is supplied.
        if (env.groundHeight && !IsBird() && !(c_.kind == Kind::Butterfly && c_.background)) {
            const float floor = env.groundHeight(pos_.x, oldBottom);
            grounded_ = std::isfinite(floor) && oldBottom <= floor + .001f && velocity_.y <= 0;
            if (grounded_) { pos_.y = floor + c_.collisionHeight * .5f - c_.collisionOffsetY; velocity_.y = 0; ay_ = 0; }
        }
        // Source adds acceleration without dt, clamps, then schedules gravity
        // acceleration for NEXT tick, moves by dt/targetDelta, then X friction.
        velocity_.x = std::clamp(velocity_.x + ax_ * dirX_, -maxX_, maxX_);
        velocity_.y = std::clamp(velocity_.y + ay_, -c_.maxVelocity.y, c_.maxVelocity.y);
        if (gravityActive_ && !grounded_) {
            const float multiplier = velocity_.y > 0 || velocity_.y < -15 ? 1.f : 3.f;
            ay_ = -c_.gravity * multiplier / currentResistanceY_ * tickScale;
        }
        if (env.hittingSolidLineEdge && c_.kind == Kind::Rabbit) { velocity_.x = 0; ax_ = 0; }
        pos_.x += velocity_.x * tickScale;
        pos_.y += velocity_.y * tickScale;
        velocity_.x *= currentResistanceX_;
        if (std::abs(velocity_.x) < .1f) velocity_.x = 0;
        // Parent's callback performs supporting-floor selection; this bounded
        // response is top contact only, not source collision-manager replacement.
        if (env.groundHeight && !IsBird() && !(c_.kind == Kind::Butterfly && c_.background)) {
            const float floor = env.groundHeight(pos_.x, oldBottom);
            grounded_ = std::isfinite(floor) && Bottom() <= floor && velocity_.y <= 0;
            if (grounded_) { pos_.y = floor + c_.collisionHeight * .5f - c_.collisionOffsetY; velocity_.y = 0; ay_ = 0; }
        } else grounded_ = false;
        // Character::Update refreshes current resistances before NPC AI.
        currentResistanceX_ = resistanceX_; currentResistanceY_ = resistanceY_;
        if (grounded_) { rising_ = falling_ = false; airTime_ = fallTime_ = 0; }
        else {
            airTime_ += dt;
            if (ay_ != 0 && velocity_.y > -.5f) { rising_ = true; falling_ = false; fallTime_ = 0; }
            else if (ay_ != 0) { rising_ = false; falling_ = true; fallTime_ += dt; }
        }
        if (IsBird()) Bird(dt, env);
        else if (c_.kind == Kind::Butterfly) Butterfly(dt, env);
        else Rabbit(dt, env);
        pose_.x = pos_.x; pose_.y = pos_.y; pose_.facing = facing_;
        return pose_;
    }
private:
    bool IsBird() const { return c_.kind == Kind::AmbientBird || c_.kind == Kind::ActiveBird; }
    float Bottom() const { return pos_.y + c_.collisionOffsetY - c_.collisionHeight * .5f; }
    std::uint32_t Random(std::uint32_t bound) {
        rng_ ^= rng_ << 13; rng_ ^= rng_ >> 17; rng_ ^= rng_ << 5;
        return bound ? rng_ % bound : 0;
    }
    float DistanceSquared(const Environment& e) const {
        float dx = e.player.x - pos_.x, dy = e.player.y - pos_.y;
        return dx * dx + dy * dy;
    }
    bool Close(const Environment& e) const { return e.hasPlayer && DistanceSquared(e) < 250.f * 250.f; }
    void AccelerateX(float direction) {
        // MovingSprite normalises sign, even source run-away's +/-3 and
        // Follow's normalised fractional X: it does NOT multiply accel by 3/X.
        if (direction == 0) return;
        dirX_ = direction < 0 ? -1.f : 1.f; ax_ = c_.accelX;
        facing_ = direction < 0 ? -1 : 1;
    }
    void Jump(float percent) {
        if (!grounded_) return; // bounded first-jump case of Character::Jump
        velocity_.y = 1; ay_ = c_.maxJumpSpeed * percent * .01f;
        grounded_ = false; rising_ = true; falling_ = false;
    }
    void PickDirection() { dirX_ = Random(2) == 0 ? 1.f : -1.f; }
    void BeginWander() {
        wander_ = WanderState::Moving; PickDirection();
        moveTime_ = 1.f + Random(100) * .01f;
        originalMaxX_ = maxX_;
    }
    void Bird(float dt, const Environment& e) {
        AccelerateX(dirX_);
        bool beyond = dirX_ > 0 ? pos_.x > c_.origin.x + c_.travelOffset
                               : pos_.x < c_.origin.x + c_.travelOffset;
        const bool viewKnown = e.visibilityKnown || (e.objectInViewAt && e.startInViewAt);
        const bool inView = e.objectInViewAt ? e.objectInViewAt(pos_) : e.objectInView;
        const bool startInView = e.startInViewAt ? e.startInViewAt(c_.origin) : e.startInView;
        if (beyond && viewKnown && !inView && !startInView &&
            e.hasPlayer && std::abs(e.player.x - pos_.x) > 1000.f) {
            pos_ = c_.origin; pose_.recycled = true; // source keeps velocity
        }
        // Preserve source's initially-zero timers and counterintuitive reset:
        // timer is randomised when LEAVING its own phase, for its NEXT entry.
        const bool active = c_.kind == Kind::ActiveBird;
        if (gliding_) {
            flapWait_ -= dt;
            if (flapWait_ <= 0) {
                flapWait_ = (active ? .35f : .9f) + Random(active ? 35 : 100) * .01f;
                gliding_ = false;
            }
        } else {
            flapEndWait_ -= dt;
            if (flapEndWait_ <= 0) {
                flapEndWait_ = (active ? 1.f : 2.f) + Random(140) * .01f;
                gliding_ = true;
            }
        }
    }
    void Butterfly(float dt, const Environment& e) {
        if (!c_.background && e.hasPlayer && DistanceSquared(e) < 130.f * 130.f) ay_ = 2.5f;
        yWait_ -= dt;
        if (yWait_ < 0 && velocity_.y < -lastBurst_) {
            ay_ = 0; lastBurst_ = (1000.f + Random(2500)) * .001f;
            velocity_.y = lastBurst_; yWait_ = .2f;
        }
        directionWait_ -= dt;
        if (directionWait_ < 0) {
            dirX_ = Random(2) == 0 ? -1.f : 1.f;
            directionWait_ = (500.f + Random(1500)) * .001f;
        }
        AccelerateX(dirX_);
    }
    void Rabbit(float dt, const Environment& e) {
        if (mode_ != RabbitMode::Wander) {
            if (!e.hasPlayer) return; // source retains prior acceleration
            const float ds = DistanceSquared(e);
            if (ds < 500.f * 500.f && (mode_ == RabbitMode::Repel || ds > stopDistance_ * stopDistance_)) {
                AccelerateX((mode_ == RabbitMode::Repel ? -1.f : 1.f) * (e.player.x - pos_.x));
                if (mode_ == RabbitMode::Follow && !e.playerOnGround && grounded_) Jump(100);
            } else ax_ = 0;
            return;
        }
        // source max/resistance update belongs to state at START of this tick.
        const bool running = wander_ == WanderState::RunningFromPlayer;
        maxX_ = originalMaxX_ * (running ? 2.f : 1.f);
        resistanceX_ = running ? .95f : .9f;
        if (wander_ == WanderState::Moving) {
            if (e.hittingSolidLineEdge) {
                if (e.hasPlayer && DistanceSquared(e) < 2.f * 250.f * 250.f) {
                    wander_ = WanderState::RunningFromPlayer; runTime_ = 2; Jump(100);
                } else dirX_ = -dirX_;
            }
            if (Close(e)) { velocity_.x = 0; wander_ = WanderState::RunningFromPlayer; return; }
            if (moveTime_ > 0) {
                moveTime_ -= dt;
                const bool inView = e.objectInViewAt ? e.objectInViewAt(pos_) : e.objectInView;
                if (!inView && ((dirX_ < 0 && pos_.x - 10 < e.levelLeft) ||
                    (dirX_ > 0 && pos_.x + 10 > e.levelRight))) dirX_ = -dirX_;
                AccelerateX(dirX_);
            } else { wander_ = WanderState::Resting; moveTime_ = 0; restTime_ = 1.f + Random(200) * .01f; }
        } else if (wander_ == WanderState::Resting) {
            if (Close(e)) { velocity_.x = 0; wander_ = WanderState::RunningFromPlayer; return; }
            if (restTime_ > 0) { restTime_ -= dt; ax_ = 0; }
            else { wander_ = WanderState::Moving; PickDirection(); restTime_ = 0; moveTime_ = 1.f + Random(100) * .01f; }
        } else {
            if (e.hittingSolidLineEdge) { runTime_ = 2; Jump(70); }
            if (runTime_ > 0) {
                if (e.hasPlayer) AccelerateX(e.player.x - pos_.x > 0 ? -3.f : 3.f);
                runTime_ -= dt;
            } else if (!Close(e)) { runTime_ = 0; wander_ = WanderState::Resting; }
            else runTime_ = 1;
        }
    }
    void Animate() {
        pose_.loop = true;
        if (IsBird()) { pose_.sequence = gliding_ ? "Still" : "Running"; pose_.rate = gliding_ ? 1.f : 9.f; }
        else if (c_.kind == Kind::Butterfly) {
            pose_.sequence = "Still"; pose_.rate = velocity_.y < -2.f ? std::abs(velocity_.y * 4.f) : 45.f;
        } else if (falling_ && (fallTime_ > .05f || airTime_ > .25f)) {
            pose_.sequence = "JumpingDown"; pose_.rate = c_.jumpingDownRate; pose_.loop = false;
        } else if (falling_) {
            // Source holds preceding sequence during the falling-delay gate.
        } else if (rising_) { pose_.sequence = "JumpingUp"; pose_.rate = c_.jumpingUpRate; pose_.loop = false; }
        else if (std::abs(velocity_.x) > .05f) {
            pose_.sequence = "Running";
            pose_.rate = std::clamp(std::abs(velocity_.x * 1.8f * 1.8f), c_.runningMinRate, c_.runningMaxRate);
        } else { pose_.sequence = "Still"; pose_.rate = c_.stillRate; }
    }
    Config c_{};
    Pose pose_{};
    Point pos_{}, velocity_{};
    std::uint32_t rng_ = 1;
    float ax_ = 0, ay_ = 0, dirX_ = 1;
    int facing_ = 1;
    float resistanceX_ = 1, resistanceY_ = 1, currentResistanceX_ = 1, currentResistanceY_ = 1;
    float maxX_ = 0, originalMaxX_ = 0, stopDistance_ = 120;
    bool gravityActive_ = false, grounded_ = false, rising_ = false, falling_ = false;
    float airTime_ = 0, fallTime_ = 0;
    bool gliding_ = false;
    float flapWait_ = 0, flapEndWait_ = 0, yWait_ = 0, lastBurst_ = 0, directionWait_ = 0;
    RabbitMode mode_ = RabbitMode::Wander;
    WanderState wander_ = WanderState::Moving;
    float moveTime_ = 0, restTime_ = 0, runTime_ = 0;
};
} // namespace deathleak::game
