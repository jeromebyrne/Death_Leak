#include "PlayerMotion.h"
#include <algorithm>
#include <cmath>

namespace deathleak::game {
namespace {
constexpr float kHalfWidth = 128.959793f / 2.f; // authored Player collision box
constexpr float kHalfHeight = 214.898743f / 2.f;
constexpr float kBaseMaxSpeed = 8.f * 60.f;
constexpr float kJumpSpeed = 21.f * 60.f;
constexpr float kGravity = (0.75f / 0.55f) * 60.f * 60.f;
// Fallback duration for motion-only callers; live gameplay clears on visual finish.
constexpr float kSoftImpactSeconds = 6.f / 30.f;
constexpr float kHardImpactSeconds = 11.f / 15.f;
constexpr float kWallJumpSpeedX = 15.f * 60.f;
constexpr float kWallJumpSpeedY = 20.f * 60.f;
}
void PlayerMotion::Reset(float x, float y, bool authoredEntry) {
    x_ = x; y_ = y; vx_ = vy_ = 0; sinceGround_ = 1; airTime_ = 0; jumpHeld_ = false; vocalActions_ = 0; grounded_ = false; initialPlacement_ = authoredEntry;
    wallSide_ = 0; wallJumpRemaining_ = 0; jumpPercent_ = 40.f; solidContactDamage_ = 0.f;
    groundedSegment_ = static_cast<size_t>(-1);
    landingApexValid_ = false; landingApexY_ = y;
    impactRecovery_ = ImpactRecovery::None; impactRecoveryTime_ = 0.f; hardLandingEvent_ = HardLandingEvent::None; landedLargeDropThisUpdate_ = false;
    downwardDashing_ = inWater_ = smashedSomething_ = false; rollTime_ = rollBuffer_ = 0.f; facing_ = 1;
}
bool PlayerMotion::TryDownwardDash(bool hasFocus) {
    if (!dashUnlocked_ || !hasFocus || grounded_ || airTime_ < 0.25f || inWater_ || downwardDashing_) return false;
    // Character::DoDownwardDash cancels vertical acceleration and sets -50 units/tick.
    vy_ = -50.f * 60.f; jumpPercent_ = 100.f; downwardDashing_ = true;
    return true;
}
bool PlayerMotion::TryRoll(bool hasFocus) {
    if (!rollUnlocked_ || !hasFocus || !grounded_ || rollTime_ > 0.f) return false;
    if (inWater_) return false;
    rollTime_ = 0.5f; // animation-finish stand-in; source Roll sequence owns termination
    if (landedLargeDropThisUpdate_ && hardLandingEvent_ == HardLandingEvent::LargeDrop)
        hardLandingEvent_ = HardLandingEvent::LargeDropRoll;
    // A successful landing roll cancels both Windows impact-recovery flags.
    impactRecovery_ = ImpactRecovery::None; impactRecoveryTime_ = 0.f;
    if (std::abs(vx_) < 12.f*60.f) vx_ = facing_ * 12.f*60.f;
    vy_ = 0.f; rollBuffer_ = 0.f;
    ++vocalActions_;
    return true;
}
bool PlayerMotion::TryBufferedRoll(bool hasFocus) {
    if (rollBuffer_ <= 0.f) return false;
    return TryRoll(hasFocus);
}
void PlayerMotion::Update(LevelScene& scene, float dt, int direction, bool jump) {
    if (dt <= 0) return;
    const float ticks = dt * 60.f;
    landedLargeDropThisUpdate_ = false;
    if (impactRecovery_ != ImpactRecovery::None && !impactVisualDriven_) {
        impactRecoveryTime_ = std::max(0.f, impactRecoveryTime_ - dt);
        if (impactRecoveryTime_ == 0.f) impactRecovery_ = ImpactRecovery::None;
    }
    solidContactDamage_ = 0.f;
    smashedSomething_ = false;
    if (rollBuffer_ > 0.f) rollBuffer_ = std::max(0.f, rollBuffer_ - dt);
    if (rollTime_ > 0.f) rollTime_ = std::max(0.f, rollTime_ - dt);
    if (direction) facing_ = direction < 0 ? -1 : 1;
    bool inWater = false;
    for (const auto& hazard : scene.Hazards())
        if (hazard.water && hazard.OverlapsPlayer(x_,y_)) inWater = true;
    inWater_ = inWater;
    for(const auto& region:scene.ForceRegions())if(std::abs(x_-region.x)<region.width*.5f && std::abs(y_-region.y)<region.height*.5f) {
        if(grounded_) {y_+=10.f;vy_=60.f;grounded_=false;}
        const float percent=(region.y+region.height*.5f-(y_-kHalfHeight))/region.height;
        vy_+=region.directionY*region.force*percent*dt*60.f;
    }
    const float maxSpeed = speedUpgrade_ ? 11.f * 60.f : kBaseMaxSpeed;
    if (wallJumpRemaining_ > 0) wallJumpRemaining_ = std::max(0.f, wallJumpRemaining_ - dt);
    if (rollTime_ > 0.f) {
        vx_ += facing_ * 60.f * ticks;
        vx_ *= std::pow(0.93f, ticks);
    } else if (wallJumpRemaining_ <= 0) {
        if (direction) vx_ = std::clamp(vx_ + direction * 60.f * ticks, -maxSpeed*(inWater ? 0.4f : 1.f), maxSpeed*(inWater ? 0.4f : 1.f));
        else vx_ *= std::pow(0.80f, ticks); // Player::UpdateResistance without acceleration
    } else vx_ *= std::pow(0.99f, ticks);
    if (std::abs(vx_) < 6.f) vx_ = 0;
    // Character::CanJump permits a 0.3 s post-edge window.
    if (inWater && jump && !downwardDashing_ && rollTime_ <= 0.f) {
        // Gamepad swim applies AccelerateY(1, 0.3) each target update.
        // MovingSprite clamps submerged vertical speed to four units/tick.
        vy_ = std::min(4.f*60.f, vy_ + 0.3f*60.f*ticks);
    } else if (jump && !jumpHeld_ && !downwardDashing_ && rollTime_ <= 0.f && (grounded_ || sinceGround_ <= 0.3f || wallSide_)) {
        const bool wallJump = !grounded_ && sinceGround_ > 0.3f && wallSide_;
        if (wallJump) {
            vx_ = -wallSide_ * kWallJumpSpeedX;
            vy_ = kWallJumpSpeedY;
            wallJumpRemaining_ = 0.3f;
        } else { vy_ = kJumpSpeed * 0.4f; jumpPercent_ = 40.f; }
        y_ += 10.f; // Character::Jump bumps Y before DoJumpParticles checks retained contact.
        if (grounded_ && groundedSegment_ < scene.Ground().size())
            scene.EmitJumpTakeoff(groundedSegment_, x_, y_, facing_);
        grounded_ = false; sinceGround_ = 1.f; airTime_ = 0.f;
        landingApexValid_ = true; landingApexY_ = y_;
        groundedSegment_ = static_cast<size_t>(-1);
        // Character::WallJump uses its separate jump.wav, not PlayJumpSFX.
        if (!wallJump) ++vocalActions_;
    }
    // InputManager::ProcessJump_* starts at 40% and increases by 6.5%
    // on each subsequent held update, up to the authored max jump speed.
    // Character::IncreaseJump only raises velocity, never lowers it.
    if (jump && jumpHeld_ && !downwardDashing_ && rollTime_ <= 0.f && !inWater && !grounded_ && sinceGround_ > 0.3f && wallJumpRemaining_ <= 0 && jumpPercent_ < 100.f) {
        jumpPercent_ = std::min(100.f, jumpPercent_ + 6.5f * ticks);
        vy_ = std::max(vy_, kJumpSpeed * jumpPercent_ / 100.f);
    }
    if (!jump) jumpPercent_ = 40.f;
    jumpHeld_ = jump;
    // Track the center high point of this actual airborne episode. The
    // authored first-frame placement is not an airborne descent.
    if (!grounded_ && !landingApexValid_ && !initialPlacement_) {
        landingApexValid_ = true; landingApexY_ = y_;
    }
    if (landingApexValid_) landingApexY_ = std::max(landingApexY_, y_);
    const float oldX = x_, oldY = y_;
    if(grounded_ && groundedSegment_<scene.Ground().size()) {
        const auto& line=scene.Ground()[groundedSegment_];
        const float nextX=x_+vx_*dt;
        if((line.hardRight && vx_>0.f && nextX>line.stripRight-line.rightOffset) ||
           (line.hardLeft && vx_<0.f && nextX<line.stripLeft+line.leftOffset))vx_=0.f;
    }
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
            { x_ = box.left-kHalfWidth; vx_ = 0; wallSide_ = 1; solidContactDamage_ = std::max(solidContactDamage_, box.damage); }
        else if (vx_ < 0 && oldX-kHalfWidth >= box.right && x_-kHalfWidth < box.right)
            { x_ = box.right+kHalfWidth; vx_ = 0; wallSide_ = -1; solidContactDamage_ = std::max(solidContactDamage_, box.damage); }
    }
    if (wallSide_ && impactRecovery_ == ImpactRecovery::Hard) {
        impactRecovery_ = ImpactRecovery::None; impactRecoveryTime_ = 0.f;
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
    size_t footstepSegment = static_cast<size_t>(-1);
    for (size_t segment = 0; segment < scene.Ground().size(); ++segment) {
        const auto& line = scene.Ground()[segment];
        const float dx = line.x2-line.x1;
        if (std::abs(dx) <= 1.f) continue;
        const float t = (x_-line.x1)/dx;
        if (t < 0 || t > 1) continue;
        const float surface = line.y1+t*(line.y2-line.y1);
        if ((wasGrounded && std::abs(surface-(oldY-kHalfHeight)) <= 20.f) ||
            (vy_ <= 0 && (oldY-kHalfHeight >= surface-4.f || (initialPlacement_ && oldY-kHalfHeight >= surface-32.f)) && y_-kHalfHeight <= surface))
            if (surface >= best) { best = surface; footstepSegment = segment; }
    }
    for (const auto& box : scene.SolidBoxes()) {
        if (x_+kHalfWidth <= box.left || x_-kHalfWidth >= box.right) continue;
        if ((wasGrounded && std::abs(box.top-(oldY-kHalfHeight)) <= 20.f) ||
            (vy_ <= 0 && (oldY-kHalfHeight >= box.top-4.f || (initialPlacement_ && oldY-kHalfHeight >= box.top-32.f)) && y_-kHalfHeight <= box.top))
            if (box.top >= best) { best = box.top; footstepSegment = static_cast<size_t>(-1); }
        // Reject upward penetration through the underside of a solid object.
        if (vy_ > 0 && oldY+kHalfHeight <= box.bottom && y_+kHalfHeight > box.bottom) {
            solidContactDamage_ = std::max(solidContactDamage_, box.damage);
            y_ = box.bottom-kHalfHeight;
            vy_ = 0;
        }
    }
    // Smashable::OnCollision only accepts a downward-dashing player entering
    // the upper half of its collision box. A normal landing remains solid.
    // The source block remains solid before the smash technique is owned.
    for (size_t i = 0; i < scene.Smashables().size(); ++i) {
        const auto& smash = scene.Smashables()[i];
        if (smash.smashed) continue;
        const auto box = smash.box;
        if (x_+kHalfWidth <= box.left || x_-kHalfWidth >= box.right) continue;
        const float previousBottom = oldY-kHalfHeight, bottom = y_-kHalfHeight;
        if (downwardDashing_ && vy_ <= 0 && previousBottom >= box.top-4.f &&
            bottom <= box.top && bottom > (box.top+box.bottom)*0.5f) {
            smashedSomething_ |= scene.SmashAt(i);
            continue;
        }
        if ((wasGrounded && std::abs(box.top-previousBottom) <= 20.f) ||
            (vy_ <= 0 && previousBottom >= box.top-4.f && bottom <= box.top))
            if (box.top >= best) { best = box.top; footstepSegment = static_cast<size_t>(-1); }
        if (vy_ > 0 && oldY+kHalfHeight <= box.bottom && y_+kHalfHeight > box.bottom) {
            y_ = box.bottom-kHalfHeight; vy_ = 0;
        }
    }
    const bool dashAtContact = downwardDashing_;
    if (best > -1e8f) { y_ = best+kHalfHeight; vy_ = 0; grounded_ = true; groundedSegment_ = footstepSegment; sinceGround_ = 0; airTime_ = 0; downwardDashing_ = false; }
    else { groundedSegment_ = static_cast<size_t>(-1); sinceGround_ += dt; airTime_ += dt; }
    if (grounded_) {
        if (!wasGrounded && !initialPlacement_) {
            // Downward-dash contact is a hard impact even for a short descent.
            // Ordinary distance impact excludes water and an existing roll.
            if (dashAtContact) {
                impactRecovery_ = ImpactRecovery::Hard;
                impactRecoveryTime_ = kHardImpactSeconds;
                hardLandingEvent_ = HardLandingEvent::DownwardDash;
            } else if (landingApexValid_ && !inWater_) {
                const float drop = landingApexY_ - y_;
                if (drop >= 700.f) {
                    if (!Rolling()) {
                        hardLandingEvent_ = HardLandingEvent::LargeDrop;
                        landedLargeDropThisUpdate_ = true;
                        impactRecovery_ = ImpactRecovery::Hard;
                        impactRecoveryTime_ = kHardImpactSeconds;
                    }
                } else if (drop > 300.f && !Rolling()) {
                    impactRecovery_ = ImpactRecovery::Soft;
                    impactRecoveryTime_ = kSoftImpactSeconds;
                }
            }
            if (landingApexValid_)
                scene.EmitLandingMaterialSpray(groundedSegment_, x_, y_, facing_, landingApexY_ - y_);
        }
        landingApexValid_ = false;
    } else if (!initialPlacement_) {
        if (!landingApexValid_) landingApexY_ = oldY;
        landingApexValid_ = true;
        landingApexY_ = std::max(landingApexY_, y_);
    }
    scene.SetPlayerFootstepContact(grounded_ ? footstepSegment : static_cast<size_t>(-1), x_, y_);
    // Authored LevelEntry centers can begin a few units inside a solid line.
    // Allow a one-frame shallow floor depenetration; ordinary jumps still
    // require crossing the surface from above.
    initialPlacement_ = false;
}
}
