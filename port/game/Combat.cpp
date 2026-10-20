#include "Combat.h"
#include <algorithm>
#include <cmath>
#include <filesystem>

namespace deathleak::game {
namespace {
bool Inside(float x, float y, float cx, float cy, float w, float h) {
    return std::abs(x-cx) <= w*0.5f && std::abs(y-cy) <= h*0.5f;
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
    hurtCooldown_ = std::max(0.f, hurtCooldown_-dt);
    for (const auto& trigger : scene.Encounters()) {
        if (!Inside(x,y,trigger.x,trigger.y,trigger.width,trigger.height) || !activeTriggers_.insert(trigger.id).second) continue;
        if (!ninjaArt_) SetArt(renderer);
        for (int i=0; i<std::clamp(trigger.count,0,20); ++i) {
            // Original spawners use a 1200x1200 box around the player. Fixed offsets make scripted runs reproducible.
            float offset = (i%2 ? -1.f : 1.f) * (150.f + 85.f*(i/2));
            auto art = trigger.type == "kGhost" ? ghostArt_ : trigger.type == "kSkeleton" ? skeletonArt_ : ninjaArt_;
            enemies_.push_back({x+offset, y+170.f, trigger.type == "kGhost" ? 50.f : trigger.type == "kSkeleton" ? 500.f : 20.f,
                0.8f+0.25f*i, trigger.type, art});
            ++spawned_;
        }
    }
    if (fire && fireCooldown_ <= 0 && health_ > 0) {
        if (!knifeArt_) SetArt(renderer);
        shots_.push_back({x+facing*10.f,y+25.f,facing*1200.f,0.f,1.2f,1.5f,false});
        fireCooldown_ = 0.220f; // Player::mProjectileFireDelay
    }
    for (auto& enemy : enemies_) {
        if (enemy.health <= 0) continue;
        const float dx=x-enemy.x, dy=y-enemy.y;
        enemy.fireCooldown -= dt;
        if (enemy.type == "kGhost") {
            enemy.x += std::clamp(dx, -210.f*dt, 210.f*dt);
            enemy.y += std::clamp(dy, -100.f*dt, 100.f*dt);
        } else {
            if (std::abs(dx)>130.f) enemy.x += std::copysign(220.f*dt,dx);
            // Ground-following and jumping still differ from original NPC/AIState.
            enemy.y += std::clamp(dy, -120.f*dt, 120.f*dt);
        }
        if (enemy.fireCooldown <= 0 && std::abs(dx)<700.f && std::abs(dy)<300.f) {
            const float length=std::max(1.f,std::hypot(dx,dy));
            const float speed=enemy.type == "kSkeleton" ? 300.f : 420.f;
            shots_.push_back({enemy.x,enemy.y+35.f,dx/length*speed,dy/length*speed,2.f,
                enemy.type == "kSkeleton" ? 35.f : 2.f,true});
            enemy.fireCooldown=enemy.type == "kSkeleton" ? 2.2f : 1.5f;
        }
    }
    for (auto& shot : shots_) {
        if (shot.life<=0) continue;
        const float oldX=shot.x, oldY=shot.y;
        shot.x+=shot.vx*dt; shot.y+=shot.vy*dt; shot.life-=dt;
        if (shot.enemy) {
            if (Inside(shot.x,shot.y,x,y,70.f,180.f) && hurtCooldown_<=0) {
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
    bool inWater = false;
    for (const auto& hazard : scene.Hazards()) {
        if (!Inside(x,y,hazard.x,hazard.y,hazard.width,hazard.height)) continue;
        if (hazard.water) inWater = true;
        else health_=std::max(0.f,health_-hazard.damage*dt);
    }
    if (inWater) {
        focus_ = std::max(0.f, focus_-0.5f*dt); // Player::kWaterFocusUseRate
        if (focus_ <= 0) health_=std::max(0.f,health_-3.f*dt); // drowning follows focus depletion
    } else {
        focus_ = std::min(100.f,focus_+10.f*dt); // Player::kFocusRechargeRate
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
