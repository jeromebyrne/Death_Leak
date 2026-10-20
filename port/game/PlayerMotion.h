#pragma once
#include "LevelScene.h"

namespace deathleak::game {
// World coordinates are y-up. Velocities below are world units/second; the
// original MovingSprite values are per 60 Hz target update.
class PlayerMotion {
public:
    void Reset(float x, float y, bool authoredEntry = false);
    void Update(const LevelScene& scene, float dt, int direction, bool jump);
    float X() const { return x_; }
    float Y() const { return y_; }
    float VelocityX() const { return vx_; }
    float VelocityY() const { return vy_; }
    bool Grounded() const { return grounded_; }
    int WallSide() const { return wallSide_; }
    float SolidContactDamage() const { return solidContactDamage_; }
private:
    float x_ = 0, y_ = 0, vx_ = 0, vy_ = 0;
    float sinceGround_ = 1;
    bool jumpHeld_ = false;
    bool grounded_ = false;
    bool initialPlacement_ = false;
    int wallSide_ = 0; // -1: wall to left, +1: wall to right
    float wallJumpRemaining_ = 0;
    float jumpPercent_ = 40.f;
    float solidContactDamage_ = 0.f; // InputManager initial 40%, +6.5% each held update
};
}
