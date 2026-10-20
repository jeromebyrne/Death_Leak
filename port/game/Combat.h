#pragma once
#include "LevelScene.h"
#include <string>
#include <algorithm>
#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace deathleak::game {
struct Enemy {
    float x, y, health, fireCooldown;
    std::string type;
    render::TextureId texture;
    float windup = 0, targetX = 0, targetY = 0;
    float vy = 0;
    bool grounded = false;
    float comboWindow = 0, stunTime = 0;
    int comboHits = 0;
    float teleportCooldown = 8.f, teleportTime = 0.f;
    float teleportX = 0.f, teleportY = 0.f;
    int teleports = 0;
};
struct Shot {
    float x, y, vx, vy, life, damage;
    bool enemy;
    float resistance = 1.f;
    bool gravity = true;
};
class Combat {
public:
    void ClearLevel() { enemies_.clear(); shots_.clear(); activeTriggers_.clear(); fireCooldown_ = burstCooldown_ = hurtCooldown_ = 0; burstCount_ = 0; }
    void SetArt(render::Renderer& renderer);
    void Update(const LevelScene& scene, float x, float y, float dt, bool fire, int facing, render::Renderer& renderer);
    bool DamageEnemy(size_t index, float damage); // shared hit path for projectiles and future melee
    void Draw(render::Renderer& renderer, float cameraX, float cameraY) const;
    void SetTriggerHistory(std::unordered_map<std::string, std::int64_t> history) { triggerHistory_ = std::move(history); }
    const std::unordered_map<std::string, std::int64_t>& TriggerHistory() const { return triggerHistory_; }
    float Health() const { return health_; }
    float Focus() const { return focus_; }
    void SetFocus(float focus) { focus_ = std::clamp(focus, 0.f, 100.f); }
    void SetVitals(float health, float focus) { health_ = std::clamp(health, 0.f, 25.f); SetFocus(focus); }
    int Hits() const { return hits_; }
    int Kills() const { return kills_; }
    int Spawned() const { return spawned_; }
    int Fired() const { return fired_; }
    size_t EnemyCount() const { return enemies_.size(); }
    size_t ShotCount() const { return shots_.size(); }
    const std::vector<Enemy>& Enemies() const { return enemies_; }
    const std::vector<Shot>& Shots() const { return shots_; }
private:
    std::vector<Enemy> enemies_;
    std::vector<Shot> shots_;
    std::unordered_set<std::string> activeTriggers_;
    std::unordered_map<std::string, std::int64_t> triggerHistory_;
    render::TextureId ninjaArt_ = 0, ghostArt_ = 0, skeletonArt_ = 0, knifeArt_ = 0, hostileArt_ = 0;
    float health_ = 25.f, focus_ = 100.f, fireCooldown_ = 0.f, burstCooldown_ = 0.f;
    float focusCooldown_ = 0.f, hurtCooldown_ = 0.f;
    int burstCount_ = 0;
    int hits_ = 0, kills_ = 0, spawned_ = 0, fired_ = 0;
};
}
