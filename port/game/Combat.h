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
    float jumpRollDelay = 2.f, rollTime = 0.f;
    int jumps = 0, rolls = 0;
    float meleeStrikeCooldown = 0.f;
    std::string visualPose = "Still";
    size_t visualFrame = 0;
    float visualElapsed = 0.f;
    int facing = 1;
    float lastVisualX = 0.f;
};
struct Shot {
    float x, y, vx, vy, life, damage;
    bool enemy;
    float resistance = 1.f;
    bool gravity = true;
    bool deflectable = true;
    bool wasDeflected = false;
    float baseSpeed = 0.f;
    float meleeCooldown = 0.f;
    render::TextureId texture = render::InvalidTexture;
    float visualWidth = 84.5f, visualHeight = 25.f;
};
class Combat {
public:
    void ClearLevel() { enemies_.clear(); shots_.clear(); activeTriggers_.clear(); fireCooldown_ = burstCooldown_ = hurtCooldown_ = 0; burstCount_ = 0; playerContacts_ = contactFrames_ = 0; meleeTime_ = 0; meleeHeld_ = false; }
    void SetArt(render::Renderer& renderer);
    void Update(const LevelScene& scene, float x, float y, float dt, bool fire, bool melee, bool grounded, bool rolling, int facing, render::Renderer& renderer);
    bool DamageEnemy(size_t index, float damage, float projectileDirectionX = 0.f); // shared projectile/melee hit path
    bool MeleeActive() const { return meleeTime_ > 0.f; }
    float MeleeElapsed() const { return meleeTime_; }
    int MeleeStrikes() const { return meleeStrikes_; }
    int Deflections() const { return deflections_; }
    void SetDeflectionUnlocked(bool unlocked) { deflectionUnlocked_ = unlocked; }
    bool DeflectionUnlocked() const { return deflectionUnlocked_; }
    void SetProjectileUpgrades(bool damage, bool speed) { projectileDamageUpgrade_ = damage; projectileSpeedUpgrade_ = speed; }
    void Draw(render::Renderer& renderer, float cameraX, float cameraY) const;
    void UpdateVisuals(float dt);
    void SetTriggerHistory(std::unordered_map<std::string, std::int64_t> history) { triggerHistory_ = std::move(history); }
    void EnableSimulatedWallClock(); // test-only: advance trigger clock by fixed gameplay dt
    const std::unordered_map<std::string, std::int64_t>& TriggerHistory() const { return triggerHistory_; }
    void ApplySolidContactDamage(float damage) { health_ = std::max(0.f, health_ - damage); }
    float Health() const { return health_; }
    float MaxHealth() const { return maxHealth_; }
    void SetMaxHealth(float value) { maxHealth_ = std::clamp(value, 1.f, 1000000.f); health_ = std::min(health_, maxHealth_); }
    float Focus() const { return focus_; }
    bool HasFocus(float amount) const { return focus_ >= amount; }
    void ConsumeFocus(float amount) { focus_ = std::max(0.f, focus_ - amount); if (focus_ == 0.f) focusCooldown_ = 5.f; }
    void SetFocus(float focus) { focus_ = std::clamp(focus, 0.f, 100.f); }
    void SetVitals(float health, float focus) { health_ = std::clamp(health, 0.f, maxHealth_); SetFocus(focus); }
    int Hits() const { return hits_; }
    int Kills() const { return kills_; }
    int Spawned() const { return spawned_; }
    int Fired() const { return fired_; }
    int PlayerContacts() const { return playerContacts_; }
    int ContactFrames() const { return contactFrames_; }
    size_t EnemyCount() const { return enemies_.size(); }
    size_t ShotCount() const { return shots_.size(); }
    const std::vector<Enemy>& Enemies() const { return enemies_; }
    const std::vector<Shot>& Shots() const { return shots_; }
private:
    std::vector<Enemy> enemies_;
    std::vector<Shot> shots_;
    std::unordered_set<std::string> activeTriggers_;
    std::unordered_map<std::string, std::int64_t> triggerHistory_;
    bool simulatedWallClock_ = false;
    std::int64_t simulatedClockOriginMs_ = 0;
    double simulatedClockElapsedMs_ = 0;
    struct VisualSequence { std::vector<render::TextureId> frames; float rate = 0; bool loop = false; };
    using VisualSet = std::unordered_map<std::string, VisualSequence>;
    VisualSet ninjaVisuals_, ghostVisuals_, skeletonVisuals_;
    render::TextureId ninjaArt_ = 0, ghostArt_ = 0, skeletonArt_ = 0, knifeArt_ = 0;
    render::TextureId ninjaShotArt_ = 0, ghostShotArt_ = 0, skeletonShotArt_ = 0, upgradedKnifeArt_ = 0;
    float maxHealth_ = 25.f, health_ = 25.f, focus_ = 100.f, fireCooldown_ = 0.f, burstCooldown_ = 0.f;
    float focusCooldown_ = 0.f, hurtCooldown_ = 0.f;
    float meleeTime_ = 0.f;
    bool meleeHeld_ = false;
    int meleeStrikes_ = 0;
    int deflections_ = 0;
    bool deflectionUnlocked_ = false;
    bool projectileDamageUpgrade_ = false, projectileSpeedUpgrade_ = false;
    int burstCount_ = 0;
    int hits_ = 0, kills_ = 0, spawned_ = 0, fired_ = 0;
    int playerContacts_ = 0, contactFrames_ = 0;
};
}
