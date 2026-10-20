#include "Combat.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>

namespace deathleak::game {
namespace {
bool Inside(float x, float y, float cx, float cy, float w, float h) {
    return std::abs(x-cx) <= w*0.5f && std::abs(y-cy) <= h*0.5f;
}
// First contact of a moving projectile center with an authored segment.
bool SegmentContact(float ax, float ay, float bx, float by,
                    float cx, float cy, float dx, float dy, float& contact) {
    const float rx = bx-ax, ry = by-ay, sx = dx-cx, sy = dy-cy;
    const float divisor = rx*sy-ry*sx;
    if (std::abs(divisor) < 0.00001f) return false;
    const float qx = cx-ax, qy = cy-ay;
    const float t = (qx*sy-qy*sx)/divisor;
    const float u = (qx*ry-qy*rx)/divisor;
    if (t < 0.f || t > 1.f || u < 0.f || u > 1.f) return false;
    contact = t;
    return true;
}
bool SweptBoxContact(float ax, float ay, float bx, float by,
                     float left, float right, float bottom, float top, float& contact) {
    const float dx=bx-ax, dy=by-ay;
    float enter=0.f, leave=1.f;
    auto slab=[&](float start,float travel,float low,float high) {
        if (std::abs(travel)<0.00001f) return start>low && start<high;
        float a=(low-start)/travel, b=(high-start)/travel;
        if (a>b) std::swap(a,b);
        enter=std::max(enter,a); leave=std::min(leave,b);
        return enter<=leave;
    };
    if (!slab(ax,dx,left,right) || !slab(ay,dy,bottom,top)) return false;
    contact=enter;
    return true;
}
bool BoxContact(float ax, float ay, float bx, float by, const SolidBox& box, float& contact) {
    if (ax > box.left && ax < box.right && ay > box.bottom && ay < box.top) { contact = 0.f; return true; }
    bool found = false;
    auto edge = [&](float cx, float cy, float dx, float dy) {
        float t;
        if (SegmentContact(ax,ay,bx,by,cx,cy,dx,dy,t) && t < contact) { contact=t; found=true; }
    };
    edge(box.left,box.bottom,box.right,box.bottom);
    edge(box.right,box.bottom,box.right,box.top);
    edge(box.right,box.top,box.left,box.top);
    edge(box.left,box.top,box.left,box.bottom);
    return found;
}
// NinjaSpawner and SkeletonEnemySpawner both give NPCs a 100x200 collision box.
// Grounded NPCs use MovingSprite's default gravity (1.0) and their spawner's
// vertical resistance (1.4), scaled from the original 60 Hz target update.
void MoveGroundEnemy(Enemy& enemy, const LevelScene& scene, float desiredX, float dt) {
    constexpr float halfW = 50.f, halfH = 100.f;
    const float oldX = enemy.x, oldY = enemy.y;
    enemy.x = desiredX;
    for (const auto& line : scene.Ground()) {
        if (std::abs(line.x2-line.x1) > 1.f) continue;
        const float wallX = (line.x1+line.x2)*0.5f;
        if (oldY+halfH <= std::min(line.y1,line.y2) || oldY-halfH >= std::max(line.y1,line.y2)) continue;
        if (desiredX > oldX && oldX+halfW <= wallX && enemy.x+halfW > wallX) enemy.x=std::min(enemy.x,wallX-halfW);
        if (desiredX < oldX && oldX-halfW >= wallX && enemy.x-halfW < wallX) enemy.x=std::max(enemy.x,wallX+halfW);
    }
    for (const auto& box : scene.SolidBoxes()) {
        if (oldY+halfH <= box.bottom || oldY-halfH >= box.top) continue;
        if (desiredX > oldX && oldX+halfW <= box.left && enemy.x+halfW > box.left) enemy.x=std::min(enemy.x,box.left-halfW);
        if (desiredX < oldX && oldX-halfW >= box.right && enemy.x-halfW < box.right) enemy.x=std::max(enemy.x,box.right+halfW);
    }
    constexpr float gravity = (1.f/1.4f)*60.f*60.f;
    if (!enemy.grounded) {
        const float acceleration = enemy.vy < 0 && enemy.vy > -15.f*60.f ? gravity*3.f : gravity;
        enemy.y += enemy.vy*dt - 0.5f*acceleration*dt*dt;
        enemy.vy = std::max(-34.f*60.f, enemy.vy-acceleration*dt);
    }
    const bool wasGrounded = enemy.grounded;
    enemy.grounded = false;
    float floor = -1e9f;
    for (const auto& line : scene.Ground()) {
        const float dx = line.x2-line.x1;
        if (std::abs(dx) <= 1.f) continue;
        const float t=(enemy.x-line.x1)/dx;
        if (t < 0.f || t > 1.f) continue;
        const float surface=line.y1+t*(line.y2-line.y1);
        if ((wasGrounded && std::abs(surface-(oldY-halfH)) <= 20.f) ||
            (enemy.vy <= 0.f && oldY-halfH >= surface-4.f && enemy.y-halfH <= surface)) floor=std::max(floor,surface);
    }
    for (const auto& box : scene.SolidBoxes()) {
        if (enemy.x+halfW <= box.left || enemy.x-halfW >= box.right) continue;
        if ((wasGrounded && std::abs(box.top-(oldY-halfH)) <= 20.f) ||
            (enemy.vy <= 0.f && oldY-halfH >= box.top-4.f && enemy.y-halfH <= box.top)) floor=std::max(floor,box.top);
        if (enemy.vy > 0.f && oldY+halfH <= box.bottom && enemy.y+halfH > box.bottom) {
            enemy.y=box.bottom-halfH; enemy.vy=0.f;
        }
    }
    if (floor > -1e8f) { enemy.y=floor+halfH; enemy.vy=0.f; enemy.grounded=true; }
}
}
void Combat::SetArt(render::Renderer& renderer) {
    std::string error;
    ninjaArt_ = renderer.LoadTexture("Game/Media/characters/enemy_1/idle_default/idle_default_000.png", error);
    ghostArt_ = renderer.LoadTexture("Game/Media/characters/banshee.png", error);
    skeletonArt_ = renderer.LoadTexture("Game/Media/characters/enemy_1/idle_default/idle_default_000.png", error); // placeholder; original skeleton animation pending
    knifeArt_ = renderer.LoadTexture("Game/Media/knife.png", error);
    hostileArt_ = renderer.LoadTexture("Game/Media/puke_temp.png", error);
}
bool Combat::DamageEnemy(size_t index, float damage, float projectileDirectionX) {
    if (index >= enemies_.size() || enemies_[index].health <= 0.f ||
        enemies_[index].teleportTime > 0.f || damage <= 0.f) return false;
    auto& enemy = enemies_[index];
    enemy.health -= damage;
    // Character::OnDamage applies 30 world units of X kickback only when a
    // projectile damages a non-player character; direct/melee hits do not.
    if (projectileDirectionX != 0.f) enemy.x += std::copysign(30.f,projectileDirectionX);
    ++hits_;
    // NPC::OnDamage counts consecutive hits while the 0.5 s window is live.
    enemy.comboWindow=0.5f;
    ++enemy.comboHits;
    if (enemy.stunTime <= 0.f && enemy.grounded && enemy.comboHits >= 6) {
        enemy.stunTime=2.f;
        enemy.windup=0.f;
    }
    if (enemy.health <= 0.f) ++kills_;
    return true;
}
void Combat::Update(const LevelScene& scene, float x, float y, float dt, bool fire, bool melee, bool grounded, int facing, render::Renderer& renderer) {
    fireCooldown_ = std::max(0.f, fireCooldown_-dt);
    burstCooldown_ = std::max(0.f, burstCooldown_-dt);
    hurtCooldown_ = std::max(0.f, hurtCooldown_-dt);
    bool inWater = false;
    for (const auto& hazard : scene.Hazards())
        if (hazard.water && Inside(x,y,hazard.x,hazard.y,hazard.width,hazard.height)) inWater = true;
    // Player::DoMeleeAttack requires solid footing, 4 focus and dry ground.
    // The four sequences in player_anim_default.xml contain 7/15/9/7 frames
    // at 60 FPS. Character::DoMeleeCollisions damages only in phase 3.
    const bool meleePressed = melee && !meleeHeld_;
    meleeHeld_ = melee;
    if (meleePressed && meleeTime_ <= 0.f && grounded && !inWater && focus_ >= 4.f && health_ > 0.f) {
        meleeTime_ = 0.000001f;
        focus_ -= 4.f;
    }
    const bool meleeStrikePhase = meleeTime_ >= 22.f/60.f && meleeTime_ < 31.f/60.f;
    for (const auto& trigger : scene.Encounters()) {
        if (!Inside(x,y,trigger.x,trigger.y,trigger.width,trigger.height) || !activeTriggers_.insert(trigger.id).second) continue;
        // NPCTrigger::Initialise removes a trigger if it last fired less than
        // 120000 ms ago. Keep wall-clock history across levels and saves.
        const auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        const auto previous = triggerHistory_.find(trigger.id);
        if (previous != triggerHistory_.end() && nowMs - previous->second < 120000) continue;
        triggerHistory_[trigger.id] = nowMs;
        if (!ninjaArt_) SetArt(renderer);
        for (int i=0; i<std::clamp(trigger.count,0,20); ++i) {
            // Original spawners use a 1200x1200 box around the player. Fixed offsets make scripted runs reproducible.
            float offset = (i%2 ? -1.f : 1.f) * (150.f + 85.f*(i/2));
            auto art = trigger.type == "kGhost" ? ghostArt_ : trigger.type == "kSkeleton" ? skeletonArt_ : ninjaArt_;
            // Source SpawnMultiple uses x within +/-600, y between +300 and +600.
            const float spawnY = y + 300.f + 60.f*(i%5);
            enemies_.push_back({x+offset, spawnY, trigger.type == "kGhost" ? 50.f : trigger.type == "kSkeleton" ? 500.f : 20.f,
                1.3f+0.1f*i, trigger.type, art, 0.f, x + (offset < 0 ? -250.f : 250.f), y+300.f});
            ++spawned_;
        }
    }
    // Player::FireWeapon blocks shots in water and fires at most five knives
    // before its 0.5-second burst pause. The authored projectile lifetime is 0.25 s.
    if (fire && meleeTime_ <= 0.f && fireCooldown_ <= 0 && burstCooldown_ <= 0 && !inWater && health_ > 0) {
        if (!knifeArt_) SetArt(renderer);
        shots_.push_back({x+facing*10.f,y+25.f,facing*1200.f,0.f,0.25f,projectileDamageUpgrade_ ? 4.f : 1.5f,false});
        fireCooldown_ = projectileSpeedUpgrade_ ? 0.220f*0.75f : 0.220f;
        ++fired_;
        if (!projectileSpeedUpgrade_ && ++burstCount_ == 5) { burstCount_ = 0; burstCooldown_ = 0.5f; }
    }
    for (auto& enemy : enemies_) {
        if (enemy.health <= 0) continue;
        enemy.meleeStrikeCooldown = std::max(0.f, enemy.meleeStrikeCooldown-dt);
        // NPC::Update expires the rolling damage combo after 0.5 s; Character
        // counts down stun while the range-attack AI remains inactive.
        enemy.comboWindow = std::max(0.f, enemy.comboWindow-dt);
        if (enemy.comboWindow == 0.f) enemy.comboHits = 0;
        enemy.stunTime = std::max(0.f, enemy.stunTime-dt);
        const float dx=x-enemy.x, dy=y-enemy.y;
        enemy.fireCooldown = std::max(0.f, enemy.fireCooldown-dt);
        const bool ghost = enemy.type == "kGhost";
        if (enemy.teleportTime > 0.f) {
            enemy.teleportTime = std::max(0.f,enemy.teleportTime-dt);
            if (enemy.teleportTime > 0.f) continue;
            enemy.x=enemy.teleportX;
            enemy.y=enemy.teleportY;
            enemy.vy=0.f;
            enemy.grounded=false;
            ++enemy.teleports;
        }
        if (enemy.rollTime > 0.f) {
            enemy.rollTime = std::max(0.f, enemy.rollTime-dt);
            // Character::Roll starts at 12 units/tick and suspends ranged AI.
            MoveGroundEnemy(enemy,scene,enemy.x+std::copysign(720.f*dt,enemy.targetX-enemy.x),dt);
            continue;
        }
        if (enemy.stunTime > 0.f) {
            enemy.windup = 0.f;
            if (!ghost) MoveGroundEnemy(enemy,scene,enemy.x,dt);
            continue;
        }
        if (ghost) {
            // AIStateFlying: seek 250-400 above the player, holding a 150-400 X offset.
            const float side = enemy.targetX < x ? -1.f : 1.f;
            enemy.targetX = x + side*250.f;
            enemy.targetY = y + 300.f;
            if (enemy.x < x-250.f) enemy.x += std::min(x-250.f-enemy.x,720.f*dt);
            else if (enemy.x > x+250.f) enemy.x -= std::min(enemy.x-x-250.f,720.f*dt);
            enemy.y += std::clamp(enemy.targetY-enemy.y,-210.f*dt,210.f*dt);
        } else {
            // AIStateRangeAttack teleports a distant NPC after 8-12 s;
            // SkeletonEnemy::CanTeleport is false. Fixed in-range values replace rand().
            enemy.teleportCooldown=std::max(0.f,enemy.teleportCooldown-dt);
            if (enemy.type != "kSkeleton" && std::abs(dx)>600.f && enemy.teleportCooldown<=0.f) {
                enemy.teleportX=x-facing*550.f;
                enemy.teleportY=y+250.f;
                enemy.teleportTime=0.8f; // Character::kTeleportTime
                enemy.teleportCooldown=8.f;
                enemy.windup=0.f;
                enemy.vy=0.f;
                continue;
            }
            // AIStateRangeAttack follows beyond 500, retreats inside 200, and holds the band.
            const float range = std::abs(dx);
            const float speed = enemy.type == "kSkeleton" ? 240.f : 480.f;
            float desiredX=enemy.x;
            if (range > 500.f) desiredX += std::copysign(std::min(range-500.f,speed*dt),dx);
            else if (range < 200.f && range > 0.01f) desiredX -= std::copysign(std::min(200.f-range,speed*dt),dx);
            // AIStateRangeAttack chooses a jump if the player is >50 above,
            // otherwise a roll, only while grounded and accelerating.
            // Use the source delay's 2 s lower bound and a fixed 18-unit/tick
            // ninja jump (NinjaSpawner randomizes 16..20) for repeatability.
            enemy.jumpRollDelay = std::max(0.f,enemy.jumpRollDelay-dt);
            if (enemy.type != "kSkeleton" && enemy.grounded &&
                desiredX-enemy.x > 0.01f && enemy.jumpRollDelay <= 0.f) {
                if (y > enemy.y+50.f) {
                    enemy.vy=18.f*60.f;
                    enemy.grounded=false;
                    ++enemy.jumps;
                } else {
                    enemy.targetX=enemy.x+10000.f;
                    enemy.rollTime=0.5f; // bounded stand-in for Roll animation duration
                    ++enemy.rolls;
                }
                enemy.jumpRollDelay=2.f;
            }
            MoveGroundEnemy(enemy,scene,desiredX,dt);
            if (enemy.rollTime > 0.f) continue;
        }
        if (ghost) {
            // AIStateFlying fires only when the player is below and horizontally near.
            if (enemy.fireCooldown > 0 || y >= enemy.y || std::abs(x-enemy.x) > 45.f) continue;
        } else {
            if (enemy.windup > 0) {
                enemy.windup = std::max(0.f,enemy.windup-dt);
                if (enemy.windup > 0) continue;
            } else {
                if (enemy.fireCooldown > 0 || std::abs(dx)>700.f || std::abs(dy)>500.f) continue;
                enemy.windup=0.25f; // AIStateRangeAttack fire warmup
                continue;
            }
        }
        const float aimX=x-enemy.x, aimY=y-enemy.y;
        const float length=std::max(1.f,std::hypot(aimX,aimY));
        const bool skeleton = enemy.type == "kSkeleton";
        const float speed = skeleton ? 300.f : ghost ? 510.f : 540.f;
        shots_.push_back({enemy.x,enemy.y+(skeleton?50.f:0.f),aimX/length*speed,aimY/length*speed,
            ghost || skeleton ? 2.f : 0.25f, skeleton ? 35.f : ghost ? 2.f : 3.f,true,
            ghost ? 0.9f : 1.f, !skeleton});
        shots_.back().deflectable=!ghost && !skeleton;
        shots_.back().baseSpeed=speed;
        enemy.fireCooldown=1.3f; // fixed within NPC's original 1.0..1.6 s randomized reload
    }
    // CollisionManager dispatches each strict-AABB character contact in both
    // directions. Character::OnCollision deliberately skips SolidMovingSprite
    // displacement for another Character, so contact alone must not move,
    // ground, knock back, or damage either participant. Record the pair for
    // deterministic diagnostics and the melee callback below.
    playerContacts_ = 0;
    for (size_t index = 0; index < enemies_.size(); ++index) {
        auto& enemy = enemies_[index];
        if (enemy.health <= 0.f || enemy.teleportTime > 0.f) continue;
        const float playerHalfW = (meleeStrikePhase ? 1.5f : 1.f)*128.959793f*0.5f;
        const float playerHalfH = (meleeTime_ > 0.f ? 1.1201479f : 1.f)*214.898743f*0.5f;
        const bool overlap = std::abs(x-enemy.x) < playerHalfW+50.f &&
                             std::abs(y-enemy.y) < playerHalfH+100.f;
        if (!overlap) continue;
        ++playerContacts_;
        // Character::OnCollision calls DoMeleeCollisions on each overlapping
        // damageable NPC. Its edge check rejects targets behind the player;
        // SolidMovingSprite allows one melee strike per target per 0.5 s.
        if (meleeStrikePhase && enemy.meleeStrikeCooldown <= 0.f &&
            ((facing > 0 && enemy.x-50.f > x-playerHalfW) ||
             (facing < 0 && enemy.x+50.f < x+playerHalfW))) {
            if (DamageEnemy(index,500.f)) {
                enemy.meleeStrikeCooldown = 0.5f;
                ++meleeStrikes_;
            }
        }
    }
    if (playerContacts_ > 0) ++contactFrames_;
    if (meleeTime_ > 0.f) {
        meleeTime_ += dt;
        if (meleeTime_ >= 38.f/60.f) meleeTime_ = 0.f;
    }
    for (auto& shot : shots_) {
        if (shot.life<=0) continue;
        shot.meleeCooldown=std::max(0.f,shot.meleeCooldown-dt);
        const float oldX=shot.x, oldY=shot.y;
        // Projectile::Update uses 60-Hz-equivalent gravity and displacement resistance.
        const float ticks=std::min(2.f,dt*60.f);
        if (shot.gravity) shot.vy -= (shot.enemy ? (shot.resistance < 1.f ? 0.5f : 0.01f) : (projectileSpeedUpgrade_ ? 0.1f : 0.25f))*60.f*ticks;
        const float newX=shot.x+shot.vx*dt*shot.resistance;
        const float newY=shot.y+shot.vy*dt*shot.resistance;
        float terrainT=1.f;
        // Original blades pass line strips while an enemy is alive, but still hit solid boxes.
        if (shot.enemy || enemies_.empty()) for (const auto& line : scene.Ground()) {
            float t;
            if (SegmentContact(oldX,oldY,newX,newY,line.x1,line.y1,line.x2,line.y2,t)) terrainT=std::min(terrainT,t);
        }
        for (const auto& box : scene.SolidBoxes()) BoxContact(oldX,oldY,newX,newY,box,terrainT);
        shot.x=oldX+(newX-oldX)*terrainT;
        shot.y=oldY+(newY-oldY)*terrainT;
        shot.life-=dt;
        if (terrainT < 1.f) shot.life=0;
        if (shot.enemy) {
            const float playerHalfW=(meleeStrikePhase?1.5f:1.f)*128.959793f*0.5f;
            const float playerHalfH=(meleeTime_>0.f?1.1201479f:1.f)*214.898743f*0.5f;
            const float shotHalfW=shot.resistance<1.f?20.f:12.5f;
            const float shotHalfH=shot.resistance<1.f?12.5f:(shot.gravity?6.5f:4.f);
            // Original ghost 40x25, ninja star 25x13, skeleton fireball 25x8.
            float contact=0.f;
            if (deflectionUnlocked_ && shot.deflectable && shot.life>0.f &&
                shot.meleeCooldown<=0.f && meleeTime_>0.f &&
                (meleeTime_<7.f/60.f || meleeStrikePhase) &&
                SweptBoxContact(oldX,oldY,shot.x,shot.y,
                                x-playerHalfW-shotHalfW,x+playerHalfW+shotHalfW,
                                y-playerHalfH-shotHalfH,y+playerHalfH+shotHalfH,contact)) {
                const float contactX=oldX+(shot.x-oldX)*contact;
                const bool front=(facing>0 && shot.vx<0.f && contactX-shotHalfW>x-playerHalfW) ||
                    (facing<0 && shot.vx>0.f && contactX+shotHalfW<x+playerHalfW);
                if (front) {
                    const float speed=shot.baseSpeed>0.f?shot.baseSpeed:std::hypot(shot.vx,shot.vy);
                    shot.x=contactX; shot.y=oldY+(shot.y-oldY)*contact;
                    shot.vx=(shot.vx<0.f?1.f:-1.f)*speed*2.5f;
                    shot.vy=0.f; // deterministic representative of source random y offset
                    shot.enemy=false; shot.damage=9999.f; shot.wasDeflected=true;
                    shot.meleeCooldown=0.5f;
                    ++deflections_;
                }
            }
            // Retain the existing prototype hitbox outside successful melee
            // deflection; upgrading all projectile collision is separate work.
            if (shot.enemy && hurtCooldown_<=0.f) {
                const float vx=shot.x-oldX, vy=shot.y-oldY;
                const float t=std::clamp(((x-oldX)*vx+(y-oldY)*vy)/(vx*vx+vy*vy+0.0001f),0.f,1.f);
                if (Inside(oldX+t*vx,oldY+t*vy,x,y,70.f,180.f)) {
                    health_=std::max(0.f,health_-shot.damage); hurtCooldown_=0.5f; shot.life=0; ++hits_;
                }
            }
        } else for (size_t enemyIndex=0; enemyIndex<enemies_.size(); ++enemyIndex) {
            auto& enemy=enemies_[enemyIndex];
            if (enemy.health<=0 || enemy.teleportTime>0.f) continue;
            // Swept segment avoids tunneling at the knife's 1200 world-units/s speed.
            const float vx=shot.x-oldX, vy=shot.y-oldY;
            const float t=std::clamp(((enemy.x-oldX)*vx+(enemy.y-oldY)*vy)/(vx*vx+vy*vy+0.0001f),0.f,1.f);
            if (Inside(oldX+t*vx,oldY+t*vy,enemy.x,enemy.y,80.f,190.f)) {
                DamageEnemy(enemyIndex,shot.damage,shot.vx); shot.life=0;
                break;
            }
        }
    }
    shots_.erase(std::remove_if(shots_.begin(),shots_.end(),[](const auto& p){return p.life<=0;}),shots_.end());
    enemies_.erase(std::remove_if(enemies_.begin(),enemies_.end(),[](const auto& e){return e.health<=0;}),enemies_.end());
    for (const auto& hazard : scene.Hazards()) {
        if (!Inside(x,y,hazard.x,hazard.y,hazard.width,hazard.height)) continue;
        if (!hazard.water) health_=std::max(0.f,health_-hazard.damage*dt);
    }
    if (inWater) {
        focus_ = std::max(0.f, focus_-0.5f*dt); // Player::kWaterFocusUseRate
        if (focus_ <= 0) focusCooldown_ = 5.f;
        if (focus_ <= 0) health_=std::max(0.f,health_-3.f*dt); // drowning follows focus depletion
    } else {
        if (focusCooldown_ > 0) focusCooldown_ = std::max(0.f,focusCooldown_-dt);
        else focus_ = std::min(100.f,focus_+10.f*dt); // Player::kFocusRechargeRate
    }
}
void Combat::Draw(render::Renderer& renderer, float cameraX, float cameraY) const {
    constexpr float scale=2.f/3.f;
    auto draw=[&](render::TextureId texture,float x,float y,float w,float h){
        if (!texture) return;
        renderer.DrawSprite({texture,{}, {640.f+(x-cameraX-w/2)*scale,360.f+(cameraY-y-h/2)*scale,w*scale,h*scale}});
    };
    for (const auto& enemy:enemies_) if (enemy.teleportTime<=0.f)
        draw(enemy.texture,enemy.x,enemy.y,enemy.type=="kGhost"?113.f:183.f,enemy.type=="kGhost"?208.f:231.f);
    for (const auto& shot:shots_) draw(shot.enemy?hostileArt_:knifeArt_,shot.x,shot.y,shot.enemy?20.f:84.5f,shot.enemy?20.f:25.f);
}
}
