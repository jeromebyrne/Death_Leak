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
}
void Combat::SetArt(render::Renderer& renderer) {
    std::string error;
    ninjaArt_ = renderer.LoadTexture("Game/Media/characters/enemy_1/idle_default/idle_default_000.png", error);
    ghostArt_ = renderer.LoadTexture("Game/Media/characters/banshee.png", error);
    skeletonArt_ = renderer.LoadTexture("Game/Media/characters/enemy_1/idle_default/idle_default_000.png", error); // placeholder; original skeleton animation pending
    knifeArt_ = renderer.LoadTexture("Game/Media/knife.png", error);
    hostileArt_ = renderer.LoadTexture("Game/Media/puke_temp.png", error);
}
void Combat::Update(const LevelScene& scene, float x, float y, float dt, bool fire, int facing, render::Renderer& renderer) {
    fireCooldown_ = std::max(0.f, fireCooldown_-dt);
    burstCooldown_ = std::max(0.f, burstCooldown_-dt);
    hurtCooldown_ = std::max(0.f, hurtCooldown_-dt);
    bool inWater = false;
    for (const auto& hazard : scene.Hazards())
        if (hazard.water && Inside(x,y,hazard.x,hazard.y,hazard.width,hazard.height)) inWater = true;
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
    if (fire && fireCooldown_ <= 0 && burstCooldown_ <= 0 && !inWater && health_ > 0) {
        if (!knifeArt_) SetArt(renderer);
        shots_.push_back({x+facing*10.f,y+25.f,facing*1200.f,0.f,0.25f,1.5f,false});
        fireCooldown_ = 0.220f;
        ++fired_;
        if (++burstCount_ == 5) { burstCount_ = 0; burstCooldown_ = 0.5f; }
    }
    for (auto& enemy : enemies_) {
        if (enemy.health <= 0) continue;
        const float dx=x-enemy.x, dy=y-enemy.y;
        enemy.fireCooldown = std::max(0.f, enemy.fireCooldown-dt);
        const bool ghost = enemy.type == "kGhost";
        if (ghost) {
            // AIStateFlying: seek 250-400 above the player, holding a 150-400 X offset.
            const float side = enemy.targetX < x ? -1.f : 1.f;
            enemy.targetX = x + side*250.f;
            enemy.targetY = y + 300.f;
            if (enemy.x < x-250.f) enemy.x += std::min(x-250.f-enemy.x,720.f*dt);
            else if (enemy.x > x+250.f) enemy.x -= std::min(enemy.x-x-250.f,720.f*dt);
            enemy.y += std::clamp(enemy.targetY-enemy.y,-210.f*dt,210.f*dt);
        } else {
            // AIStateRangeAttack follows beyond 500, retreats inside 200, and holds the band.
            const float range = std::abs(dx);
            const float speed = enemy.type == "kSkeleton" ? 240.f : 480.f;
            if (range > 500.f) enemy.x += std::copysign(std::min(range-500.f,speed*dt),dx);
            else if (range < 200.f && range > 0.01f) enemy.x -= std::copysign(std::min(200.f-range,speed*dt),dx);
            // Character terrain and jumping are not yet ported for NPCs.
            enemy.y += std::clamp(dy,-120.f*dt,120.f*dt);
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
        enemy.fireCooldown=1.3f; // fixed within NPC's original 1.0..1.6 s randomized reload
    }
    for (auto& shot : shots_) {
        if (shot.life<=0) continue;
        const float oldX=shot.x, oldY=shot.y;
        // Projectile::Update uses 60-Hz-equivalent gravity and displacement resistance.
        const float ticks=std::min(2.f,dt*60.f);
        if (shot.gravity) shot.vy -= (shot.enemy ? (shot.resistance < 1.f ? 0.5f : 0.01f) : 0.25f)*60.f*ticks;
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
            const float vx=shot.x-oldX, vy=shot.y-oldY;
            const float t=std::clamp(((x-oldX)*vx+(y-oldY)*vy)/(vx*vx+vy*vy+0.0001f),0.f,1.f);
            if (Inside(oldX+t*vx,oldY+t*vy,x,y,70.f,180.f) && hurtCooldown_<=0) {
                health_=std::max(0.f,health_-shot.damage); hurtCooldown_=0.5f; shot.life=0; ++hits_;
            }
        } else for (auto& enemy : enemies_) {
            if (enemy.health<=0) continue;
            // Swept segment avoids tunneling at the knife's 1200 world-units/s speed.
            const float vx=shot.x-oldX, vy=shot.y-oldY;
            const float t=std::clamp(((enemy.x-oldX)*vx+(enemy.y-oldY)*vy)/(vx*vx+vy*vy+0.0001f),0.f,1.f);
            if (Inside(oldX+t*vx,oldY+t*vy,enemy.x,enemy.y,80.f,190.f)) {
                enemy.health-=shot.damage; shot.life=0; ++hits_;
                if (enemy.health<=0) ++kills_;
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
    for (const auto& enemy:enemies_) draw(enemy.texture,enemy.x,enemy.y,enemy.type=="kGhost"?113.f:183.f,enemy.type=="kGhost"?208.f:231.f);
    for (const auto& shot:shots_) draw(shot.enemy?hostileArt_:knifeArt_,shot.x,shot.y,shot.enemy?20.f:84.5f,shot.enemy?20.f:25.f);
}
}
