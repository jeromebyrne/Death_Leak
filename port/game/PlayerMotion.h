#pragma once
#include "LevelScene.h"

namespace deathleak::game {
// World coordinates are y-up. Velocities below are world units/second; the
// original MovingSprite values are per 60 Hz target update.
class PlayerMotion {
public:
    using ImpactRecovery = PlayerImpactRecovery;
    enum class HardLandingEvent { None, DownwardDash, LargeDrop, LargeDropRoll };
    void Reset(float x, float y, bool authoredEntry = false);
    void SetSpeedUpgrade(bool unlocked) { speedUpgrade_ = unlocked; }
    void SetMovementUnlocks(bool dash, bool roll) { dashUnlocked_ = dash; rollUnlocked_ = roll; }
    bool TryDownwardDash(bool hasFocus);
    bool TryRoll(bool hasFocus);
    void BufferRoll() { rollBuffer_ = 0.25f; }
    bool TryBufferedRoll(bool hasFocus);
    bool DownwardDashing() const { return downwardDashing_; }
    bool Rolling() const { return rollTime_ > 0.f; }
    ImpactRecovery Recovery() const { return impactRecovery_; }
    bool ImpactRecovering() const { return impactRecovery_ != ImpactRecovery::None; }
    void SetImpactVisualDriven(bool enabled) { impactVisualDriven_ = enabled; }
    void CompleteImpactRecovery() { impactRecovery_ = ImpactRecovery::None; impactRecoveryTime_ = 0.f; }
    HardLandingEvent ConsumeHardLandingEvent() { const auto event = hardLandingEvent_; hardLandingEvent_ = HardLandingEvent::None; return event; }
    void Update(LevelScene& scene, float dt, int direction, bool jump);
    bool SmashedSomething() const { return smashedSomething_; }
    float X() const { return x_; }
    float Y() const { return y_; }
    float VelocityX() const { return vx_; }
    float VelocityY() const { return vy_; }
    bool Grounded() const { return grounded_; }
    bool InWater() const { return inWater_; }
    int WallSide() const { return wallSide_; }
    float SolidContactDamage() const { return solidContactDamage_; }
private:
    float x_ = 0, y_ = 0, vx_ = 0, vy_ = 0;
    bool speedUpgrade_ = false, dashUnlocked_ = false, rollUnlocked_ = false;
    bool downwardDashing_ = false, inWater_ = false;
    bool smashedSomething_ = false;
    float rollTime_ = 0.f, rollBuffer_ = 0.f;
    int facing_ = 1;
    float sinceGround_ = 1, airTime_ = 0;
    bool jumpHeld_ = false;
    bool grounded_ = false;
    size_t groundedSegment_ = static_cast<size_t>(-1);
    bool initialPlacement_ = false;
    bool landingApexValid_ = false;
    float landingApexY_ = 0.f;
    ImpactRecovery impactRecovery_ = ImpactRecovery::None;
    HardLandingEvent hardLandingEvent_ = HardLandingEvent::None;
    bool landedLargeDropThisUpdate_ = false;
    float impactRecoveryTime_ = 0.f;
    bool impactVisualDriven_ = false;
    int wallSide_ = 0; // -1: wall to left, +1: wall to right
    float wallJumpRemaining_ = 0;
    float jumpPercent_ = 40.f;
    float solidContactDamage_ = 0.f; // InputManager initial 40%, +6.5% each held update
};
}
