#pragma once
#include "../render/Renderer.h"
#include "../audio/SoundPlayer.h"
#include "ParticleSpray.h"
#include "ShaderClock.h"
#include <cstddef>
#include <cmath>
#include <functional>
#include <unordered_map>
#include <string>
#include <unordered_set>
#include <vector>

namespace deathleak::game {
struct SceneSprite {
    render::TextureId texture = render::InvalidTexture;
    std::string xmlId; // Source level ID, retained only for opt-in render diagnostics.
    size_t order = 0;
    float x = 0, y = 0, width = 0, height = 0;
    float rotation = 0;
    int depth = 0;
    render::Color tint{255, 255, 255, 255};
    float alpha = 1.f, alphaWhenOccluding = .5f, alphaWhenNotOccluding = 1.f;
    // ScrollingSprite::Update uses a per-object cycle and the XML alpha captured
    // by DrawableObject::Initialise; the endpoint tick retains its last draw.
    bool scrolling = false;
    float scrollStartX = 0, scrollStartY = 0, scrollDistanceX = 0, scrollDistanceY = 0;
    float scrollTime = 0, scrollElapsed = 0, scrollFadeIn = 0, scrollFadeOut = 0;
    float scrollOriginalAlpha = 1.f;
    bool fadeWhenPlayerOccluded = false, updateable = true, alwaysUpdate = false;
    bool flipX = false, flipY = false, repeatX = false;
    render::FoliageSway foliageSway;
    render::PixelWobble pixelWobble;
    bool player = false;
    bool playerShadow = false; // Runtime-created Character shadow at kGround.
    bool playerAimLine = false; // Runtime-created Player child at kPlayer.
    bool parallaxLayer = false, followCameraX = false, followCameraY = false;
    float cameraParallaxX = 0, cameraParallaxY = 0;
    bool autoScrollY = false;
    float autoScrollYSpeed = 0, autoScrollYPhase = 0;
    bool currencyOrb = false;
    bool authoredSmallOrb = false;
    float orbOriginY = 0, orbPhase = 0, orbHoverSpeed = 2, orbHoverOffsetY = 20;
    bool orbTracking = false, orbHoverEnabled = true;
    float orbTrackingTime = 0, orbVx = 0, orbVy = 0, orbAx = 0, orbAy = 0;
    float orbDirectionX = 0, orbDirectionY = 0;
    bool keyPickup = false;
    bool paperPickup = false;
    bool healthDevil = false;
    bool smashable = false;
    render::TextureId smashedTexture = render::InvalidTexture;
    std::vector<render::TextureId> animationFrames;
    float animationFrameRate = 0, animationElapsed = 0;
    size_t animationFrame = 0;
    bool dojoScroll = false;
    bool collected = false;
    std::string persistentId;
};

// Runtime GameObjects use the same descending depth stream as authored sprites.
struct PainterItem { int depth; std::function<void()> draw; };

struct GroundSegment { float x1, y1, x2, y2; std::string material; };
struct SolidBox { float left, right, bottom, top; float damage = 0; };
struct LevelTrigger { float x, y, width, height; std::string destination; };
struct EncounterTrigger { float x, y, width, height; int count; std::string type; std::string id; };
struct Door { float x, y, width, height; std::string destination, requiredKey, id; int depth = 3000; size_t order = 0; bool updateable = true, alwaysUpdate = false; };
struct Boat { float x, y, width, height; std::string id; int depth = 3000; size_t order = 0; bool updateable = true, alwaysUpdate = false; };
struct DojoScroll { float x, y, width, height; std::string feature; std::string id; int depth = 3000; size_t order = 0; bool updateable = true, alwaysUpdate = false; };
struct KeyPickup { float x, y, width, height; std::string keyId; int depth = 3000; size_t order = 0; bool updateable = true, alwaysUpdate = false; };
struct PaperPickup { float x, y, width, height; std::string descriptionId, titleId; int depth = 3000; size_t order = 0; bool updateable = true, alwaysUpdate = false; };
struct HealthDevil { float x, y, width, height; int depth = 3000; size_t order = 0; bool updateable = true, alwaysUpdate = false; };
struct Smashable { SolidBox box; float x = 0, y = 0; bool smashed = false; };
struct SpawnedOrb { float x, y, vx, vy, spawnDelay, trackingTime = 0; };
struct HazardRegion { float x, y, width, height; bool water; float damage; };

struct PlayerStrafeVisual { bool active = false; int aimFacing = 1, movementDirection = 1; };

// InputManager's raw XInput stick test; eligibility is recalculated each tick.
inline PlayerStrafeVisual ResolvePlayerStrafeVisual(render::ControllerSticks sticks,
        int movementDirection, bool rolling, bool melee, bool justLanded) {
    PlayerStrafeVisual visual{false, 1, movementDirection};
    constexpr int deadzone = 8689;
    if (!sticks.connected || (std::abs(sticks.rightX) <= deadzone && std::abs(sticks.rightY) <= deadzone) ||
        rolling || melee || justLanded) return visual;
    if (sticks.rightX <= 0 && sticks.leftX > deadzone) {
        visual.active = true; visual.aimFacing = -1;
    } else if (sticks.rightX > 0 && sticks.leftX < -deadzone) {
        visual.active = true; visual.aimFacing = 1;
    }
    return visual;
}

class LevelScene {
public:
    bool Load(const std::string& path, render::Renderer& renderer, std::string& error,
              const std::string& fromLevel = {}, const std::string& entryDoorId = {});
    void Draw(render::Renderer& renderer, float cameraX, float cameraY, float playerX, float playerY,
              std::vector<PainterItem> dynamic = {}, float playerDamageBurstGain = 1.f,
              float foliageGlobalTimeMultiplier = 1.f, ShaderClockSnapshot shaderClock = {},
              bool suppressLandingSprayDraw = false) const;
    void UpdatePlayerVisual(float dt, float vx, float vy, bool grounded, int wallSide,
                            bool rolling, bool dashing, bool inWater, float meleeElapsed, int facing,
                            bool projectileFired, bool eligibleFireAttempt = false,
                            PlayerStrafeVisual strafe = {});
    void UpdateVisuals(float dt, float playerX, float playerY, float cameraX, float cameraY,
                       bool objectsPaused = false);
    void UpdateAuthoredOrbs(float dt, float playerX, float playerY, float cameraX, float cameraY);
    size_t CollectCurrencyOrbs(float playerX, float playerY, std::unordered_set<std::string>& collected);
    void ApplyCollected(const std::unordered_set<std::string>& collected);
    bool AllowsWeather() const { return allowsWeather_; }
    float CameraX() const { return cameraX_; }
    float CameraY() const { return cameraY_; }
    float CameraOffsetX() const { return cameraOffsetX_; }
    float CameraOffsetY() const { return cameraOffsetY_; }
    float CameraLagX() const { return cameraLagX_; }
    float CameraLagY() const { return cameraLagY_; }
    float CameraZoom() const { return cameraZoom_; }
    bool CameraFollowX() const { return cameraFollowX_; }
    bool CameraFollowY() const { return cameraFollowY_; }
    bool HasCameraBounds() const { return cameraBounds_; }
    float CameraBoundLeft() const { return cameraBoundLeft_; }
    float CameraBoundRight() const { return cameraBoundRight_; }
    float CameraBoundTop() const { return cameraBoundTop_; }
    float CameraBoundBottom() const { return cameraBoundBottom_; }
    size_t SpriteCount() const { return originalSpriteCount_; }
    size_t SkippedCount() const { return skipped_; }
    float PlayerX() const { return playerX_; }
    float PlayerY() const { return playerY_; }
    float PlayerVisualWidth() const { return playerVisualWidth_; }
    float PlayerVisualHeight() const { return playerVisualHeight_; }
    const std::vector<GroundSegment>& Ground() const { return ground_; }
    void SetPlayerFootstepContact(size_t segment, float x, float y);
    void EmitJumpTakeoff(size_t segment, float x, float postBumpY, int facing);
    void EmitLandingMaterialSpray(size_t segment, float x, float y, int facing, float drop);
    const std::vector<ParticleSpray>& LandingSprays() const { return landingSprays_; }
    const std::vector<ParticleSpray>& JumpTakeoffSprays() const { return jumpTakeoffSprays_; }
    const std::vector<ParticleSpray>& FootstepSprays() const { return footstepSprays_; }
    const std::vector<SolidBox>& SolidBoxes() const { return solidBoxes_; }
    const std::vector<LevelTrigger>& Triggers() const { return triggers_; }
    const std::vector<EncounterTrigger>& Encounters() const { return encounters_; }
    const std::vector<HazardRegion>& Hazards() const { return hazards_; }
    const std::vector<audio::AmbientSource>& Ambience() const { return ambience_; }
    const std::vector<Door>& Doors() const { return doors_; }
    const std::vector<Boat>& Boats() const { return boats_; }
    const std::vector<KeyPickup>& Keys() const { return keys_; }
    const std::vector<PaperPickup>& Papers() const { return papers_; }
    const std::vector<HealthDevil>& HealthDevils() const { return healthDevils_; }
    const std::vector<Smashable>& Smashables() const { return smashables_; }
    struct OrbTrailView { std::string owner; bool attached; const ParticleSpray* spray; };
    std::vector<OrbTrailView> OrbTrails() const;
    bool SmashAt(size_t index);
    size_t UpdateSpawnedOrbs(float playerX, float playerY, float dt);
    size_t PendingSpawnedOrbs() const { return spawnedOrbs_.size(); }
    void ApplySmashable(bool smashed);
    const std::vector<DojoScroll>& Scrolls() const { return scrolls_; }
    void ApplyDeflection(bool unlocked);
    void ApplyKeys(const std::unordered_set<std::string>& keys);
    void ApplyPapers(const std::unordered_set<std::string>& papers);
    void ApplyHealthDevil(bool rewarded);
private:
    std::vector<SceneSprite> sprites_;
    std::vector<ParticleSpray> particleSprays_;
    std::vector<ParticleSpray> footstepSprays_;
    std::vector<ParticleSpray> jumpTakeoffSprays_;
    std::vector<ParticleSpray> landingSprays_;
    std::unordered_map<std::string, render::TextureId> groundParticleTextures_;
    size_t playerFootstepSegment_ = static_cast<size_t>(-1);
    float playerFootstepX_ = 0, playerFootstepY_ = 0;
    std::uint32_t footstepRandomState_ = 0x464f4f54u;
    std::uint32_t jumpRandomState_ = 0x4a554d50u;
    std::uint32_t landingRandomState_ = 0x4c414e44u;
    struct OrbTrail { std::string owner; ParticleSpray spray; bool attached = true; };
    std::vector<OrbTrail> orbTrails_;
    render::TextureId orbShatterTexture_ = render::InvalidTexture;
    void DetachOrbTrail(const std::string& owner);
    void AddOrbTrail(const SceneSprite& orb);
    std::vector<GroundSegment> ground_;
    std::vector<SolidBox> solidBoxes_;
    std::vector<LevelTrigger> triggers_;
    std::vector<EncounterTrigger> encounters_;
    std::vector<HazardRegion> hazards_;
    std::vector<audio::AmbientSource> ambience_;
    std::vector<Door> doors_;
    std::vector<Boat> boats_;
    std::vector<KeyPickup> keys_;
    std::vector<PaperPickup> papers_;
    std::vector<HealthDevil> healthDevils_;
    std::vector<Smashable> smashables_;
    std::vector<SpawnedOrb> spawnedOrbs_;
    render::TextureId spawnedOrbTexture_ = render::InvalidTexture;
    std::vector<DojoScroll> scrolls_;
    float playerX_ = 0, playerY_ = 0;
    float playerVisualWidth_ = 0, playerVisualHeight_ = 0;
    struct PlayerSequence { std::vector<render::TextureId> frames; std::vector<bool> footstepFrames; float rate = 0, minRate = 0, maxRate = 60, currentRate = 0; };
    void EmitRunningFootstep(const PlayerSequence& sequence);
    std::unordered_map<std::string, PlayerSequence> playerSequences_;
    std::unordered_map<std::string, PlayerSequence> playerArmSequences_;
    std::string playerSequence_;
    std::string playerArmSequence_;
    size_t playerFrame_ = 0;
    size_t lastRunningFrame_ = static_cast<size_t>(-1);
    size_t playerArmFrame_ = 0; // sequence length means finished, matching AnimationPart::IsFinished
    float playerFrameElapsed_ = 0;
    // Character stamps the first dry descending update; off-solid time can bypass the hold.
    double playerOffSolidTime_ = 0, playerDescentTime_ = 0;
    bool playerDescending_ = false;
    float playerArmFrameElapsed_ = 0;
    int playerFacing_ = 1;
    int playerAimLineFacing_ = 1;
    bool playerGrounded_ = false;
    float aimLineHold_ = 0.f, aimLineAlpha_ = 0.f;
    float cameraX_ = 0, cameraY_ = 0;
    float cameraOffsetX_ = 0, cameraOffsetY_ = 0;
    float cameraLagX_ = 7.5f, cameraLagY_ = 15.f, cameraZoom_ = 1.f;
    bool cameraFollowX_ = true, cameraFollowY_ = true;
    bool allowsWeather_ = false;
    bool cameraBounds_ = false;
    float cameraBoundLeft_ = 0, cameraBoundRight_ = 0, cameraBoundTop_ = 0, cameraBoundBottom_ = 0;
    size_t skipped_ = 0;
    size_t originalSpriteCount_ = 0; // Excludes runtime-created player shadow.
};
}
