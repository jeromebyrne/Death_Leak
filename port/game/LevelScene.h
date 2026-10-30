#pragma once
#include "../render/Renderer.h"
#include "../audio/SoundPlayer.h"
#include "ParticleSpray.h"
#include "AuthoredPlatformMotion.h"
#include "AuthoredAmbientActor.h"
#include "ShaderClock.h"
#include <cstddef>
#include <cstdint>
#include <cmath>
#include <functional>
#include <unordered_map>
#include <string>
#include <unordered_set>
#include <vector>

namespace deathleak::game {
enum class PlayerImpactRecovery { None, Soft, Hard };
struct SceneSprite {
    render::TextureId texture = render::InvalidTexture;
    std::string xmlId; // Source level ID, retained only for opt-in render diagnostics.
    size_t order = 0;
    std::uint64_t stableIdentity = 0; // Scene-epoch scoped creation identity.
    float x = 0, y = 0, width = 0, height = 0;
    float followWorldX = 0; // Parallax world-matrix X from the preceding eligible update.
    float rotation = 0, autoRotation = 0;
    bool sineEnabled = false;
    float sineOriginX = 0, sineOriginY = 0, sineOffsetX = 0, sineOffsetY = 0;
    float sinePhase = 0, sineSpeed = 0;
    size_t solidBoxIndex = static_cast<size_t>(-1);
    size_t platformIndex = static_cast<size_t>(-1);
    size_t ambientActorIndex = static_cast<size_t>(-1);
    int depth = 0;
    render::Color tint{255, 255, 255, 255};
    float alpha = 1.f, alphaWhenOccluding = .5f, alphaWhenNotOccluding = 1.f;
    // ScrollingSprite::Update uses a per-object cycle and the XML alpha captured
    // by DrawableObject::Initialise; the endpoint tick retains its last draw.
    bool scrolling = false;
    float scrollStartX = 0, scrollStartY = 0, scrollDistanceX = 0, scrollDistanceY = 0;
    float scrollTime = 0, scrollElapsed = 0, scrollFadeIn = 0, scrollFadeOut = 0;
    float scrollOriginalAlpha = 1.f;
    float scrollWorldX = 0, scrollWorldY = 0; // ScrollingSprite world matrix precedes its XY lerp.
    bool collisionFoliage = false; float foliageWindDown = 0.f;
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
    float autoScrollYSpeed = 0;
    float autoScrollYPhase = 0; // Committed once per selected presentation.
    float autoScrollYStep = 0; // Unwrapped delta committed by that presentation.
    bool currencyOrb = false;
    bool largeOrb = false;
    float collisionWidth = 0, collisionHeight = 0, collisionOffsetX = 0, collisionOffsetY = 0;
    bool authoredSmallOrb = false;
    float orbOriginY = 0, orbPhase = 0, orbHoverSpeed = 2, orbHoverOffsetY = 20;
    bool orbTracking = false, orbHoverEnabled = true;
    float orbTrackingTime = 0, orbVx = 0, orbVy = 0, orbAx = 0, orbAy = 0;
    float orbDirectionX = 0, orbDirectionY = 0;
    bool keyPickup = false;
    bool paperPickup = false;
    bool healthDevil = false;
    bool smashable = false;
    size_t breakableIndex = static_cast<size_t>(-1);
    std::vector<render::TextureId> damageFrames;
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

struct GroundSegment { float x1,y1,x2,y2; std::string material;
    bool hardLeft=false,hardRight=false;
    float stripLeft=0,stripRight=0,leftOffset=0,rightOffset=0;
};
struct SolidBox { float left, right, bottom, top; float damage = 0; };
struct LevelTrigger { float x, y, width, height; std::string destination; };
struct EncounterTrigger { float x, y, width, height; int count; std::string type; std::string id; };
struct Door { float x, y, width, height; std::string destination, requiredKey, id; int depth = 3000; size_t order = 0; bool updateable = true, alwaysUpdate = false; };
struct ForceRegion {float x,y,width,height,force,directionY;};
struct HealingSpring { float x,y,width,height; bool used=false; float rearm=0; };
struct Instrument { float x,y,width,height; std::string id; int depth=4500; size_t order=0; bool updateable=true,alwaysUpdate=false; float cooldown=0; };
struct Boat { float x, y, width, height; std::string id; int depth = 3000; size_t order = 0; bool updateable = true, alwaysUpdate = false; };
struct DojoScroll { float x, y, width, height; std::string feature; std::string id; int depth = 3000; size_t order = 0; bool updateable = true, alwaysUpdate = false; };
struct KeyPickup { float x, y, width, height; std::string keyId; int depth = 3000; size_t order = 0; bool updateable = true, alwaysUpdate = false; };
struct PaperPickup { float x, y, width, height; std::string descriptionId, titleId; int depth = 3000; size_t order = 0; bool updateable = true, alwaysUpdate = false; };
struct HealthDevil { float x, y, width, height; int depth = 3000; size_t order = 0; bool updateable = true, alwaysUpdate = false; };
struct Smashable { SolidBox box; float x = 0, y = 0; bool smashed = false; };
struct BreakableBody { SolidBox box; float x=0,y=0,health=10,maxHealth=10; bool canDamage=true,deleteOnBreak=false; std::string id,destroySound; };
struct SpawnedOrb { float x, y, vx, vy, spawnDelay, trackingTime = 0; };
struct HazardRegion { float x, y, width, height; bool water; float damage;
    bool OverlapsPlayer(float px,float py) const {
        return std::abs(px-x) < (width+128.959793f)*.5f &&
               std::abs(py-y) < (height+214.898743f)*.5f;
    }
};

struct PlayerStrafeVisual { bool active = false; int aimFacing = 1, movementDirection = 1; };

// InputManager writes Camera2D's target after the camera has followed for this
// update. Keep authored offsets separate from the mutable, next-follow target.
struct RightStickCameraTarget {
    float authoredX, authoredY, x, y;
    RightStickCameraTarget(float originalX, float originalY)
        : authoredX(originalX), authoredY(originalY), x(originalX), y(originalY) {}

    // SDL RIGHTY is screen-down positive; XInput RY is world-up positive.
    // SDL -32768 becomes +32767 (the largest representable XInput short).
    static int WorldRightY(int sdlRightY) {
        return sdlRightY == -32768 ? 32767 : -sdlRightY;
    }

    void Update(render::ControllerSticks sticks, bool gameplayEligible,
                bool rolling, bool falling, bool melee) {
        // Windows skips disconnected/uncontrollable input: keep the last target.
        if (!gameplayEligible || !sticks.connected) return;
        constexpr int deadzone = 8689;
        const int ry = WorldRightY(sticks.rightY);
        if (std::abs(sticks.rightX) <= deadzone && std::abs(ry) <= deadzone) {
            x = authoredX; y = authoredY;
            return;
        }
        // Aim/Y precedes strafe/X, including during a roll/fall/melee.
        y = authoredY + 300.f * static_cast<float>(ry) / 32767.f;
        if (!rolling && !falling && !melee)
            x = authoredX + 450.f * static_cast<float>(sticks.rightX) / 32767.f;
    }
};

struct CameraFollowInput {
    float cameraX, cameraY, playerX, playerY, zoom, lagX, lagY;
    bool followX, followY, enemies, bounded = false;
    float left = 0, right = 0, top = 0, bottom = 0;
};

// Follow with the stored target first, then Update it from this frame's input.
inline void FollowCameraWithTarget(CameraFollowInput input,
                                   const RightStickCameraTarget& target,
                                   float& cameraX, float& cameraY) {
    cameraX = input.cameraX; cameraY = input.cameraY;
    const auto clamp = [&] {
        if (!input.bounded) return;
        if (cameraY - 540.f <= input.bottom) cameraY = input.bottom + 540.f;
        else if (cameraY + 540.f >= input.top) cameraY = input.top - 540.f;
        if (cameraX - 960.f <= input.left) cameraX = input.left + 960.f;
        else if (cameraX + 960.f >= input.right) cameraX = input.right - 960.f;
    };
    clamp();
    if (input.followX) {
        const float lag = std::max(1.f, (input.enemies ? 40.f : input.lagX) * input.zoom);
        cameraX += (input.playerX + target.x * input.zoom - cameraX) / lag;
    }
    if (input.followY) {
        const float lag = std::max(1.f, input.lagY);
        cameraY += (input.playerY + target.y * input.zoom - cameraY) / lag;
    }
    clamp();
}

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
    enum class IntroBodyPhase { None, Looped, Pull };
    // Caller owns fresh-save eligibility and held-Interact threshold. The scene
    // owns only the selected body sequence and its animation terminal signal.
    void SetIntroBodyPhase(IntroBodyPhase phase);
    void EmitIntroSwordPullBurst(render::Renderer& renderer, float playerX, float playerY);
    const std::vector<ParticleSpray>& IntroSwordPullSprays() const { return introSwordPullSprays_; }
    IntroBodyPhase GetIntroBodyPhase() const { return introBodyPhase_; }
    bool IntroBodyFinished() const { return introBodyPhase_ == IntroBodyPhase::Pull && introBodyFinished_; }
    size_t IntroBodyFrame() const { return playerFrame_; }
    size_t IntroBodyFrameCount() const;
    std::uint64_t SceneEpoch() const { return sceneEpoch_; }
    bool Load(const std::string& path, render::Renderer& renderer, std::string& error,
              const std::string& fromLevel = {}, const std::string& entryDoorId = {});
    void PreparePresentation(); // Commit former draw-time parallax/prior-render state once.
    void Draw(render::Renderer& renderer, float cameraX, float cameraY, float playerX, float playerY,
              std::vector<PainterItem> dynamic = {}, float playerDamageBurstGain = 1.f,
              float foliageGlobalTimeMultiplier = 1.f, ShaderClockSnapshot shaderClock = {},
              bool suppressLandingSprayDraw = false,
              std::function<void()> afterPlayerDraw = {},
              std::function<void(const SceneSprite&, const render::Sprite&)> emittedOrdinarySprite = {},
              std::function<void(const SceneSprite&, const render::Sprite&, bool)> emittedAnySprite = {},
              std::function<void(const ParticleSpray&, const ParticleSpray::Particle&, const render::Sprite&)> emittedParticle = {}) const;
    void UpdatePlayerVisual(float dt, float vx, float vy, bool grounded, int wallSide,
                            bool rolling, bool dashing, bool inWater, float meleeElapsed, int facing,
                            bool projectileFired, bool eligibleFireAttempt = false,
                            PlayerStrafeVisual strafe = {}, bool rightStickAim = false,
                            float aimX = 0.f, float aimY = 0.f, float precedingRenderedModifiedDelta = -1.f,
                            bool crouch = false, PlayerImpactRecovery impactRecovery = PlayerImpactRecovery::None);
    bool ImpactVisualFinished() const { return impactVisualFinished_; }
    bool UpdateHealing(float dt,float playerX,float playerY,bool fullyCrouched);
    bool FullyCrouched() const;
    const std::vector<ForceRegion>& ForceRegions() const { return forceRegions_; }
    const std::vector<HealingSpring>& HealingSprings() const { return healingSprings_; }
    const std::vector<Instrument>& Instruments() const { return instruments_; }
    bool PlayInstrument(const std::string& id);
    std::vector<std::string> ConsumeSceneSounds() { auto sounds=std::move(sceneSounds_);sceneSounds_.clear();return sounds; }
    void SetPlayerVelocity(float x,float y) { playerVelocityX_=x;playerVelocityY_=y; }
    float UpdateDynamics(float dt,float playerX,float playerY,float cameraX,float cameraY,bool grounded=false,float velocityY=0.f);
    void UpdateVisuals(float dt, float playerX, float playerY, float cameraX, float cameraY,
                       bool objectsPaused = false, float precedingRenderedModifiedDelta = -1.f);
    void UpdateAuthoredOrbs(float dt, float playerX, float playerY, float cameraX, float cameraY);
    void SpawnTemporaryOrbs(float x, float y, unsigned count, unsigned seed);
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
    float PlayerEntryDirection() const { return playerEntryDirection_; }
    float PlayerVisualWidth() const { return playerVisualWidth_; }
    float PlayerVisualHeight() const { return playerVisualHeight_; }
    const std::vector<ParticleSpray>& HealthDevilSprays() const { return healthDevilSprays_; }
    const std::vector<ParticleSpray>& AuthoredParticleSprays() const { return particleSprays_; }
    const std::vector<SceneSprite>& SceneSprites() const { return sprites_; }
    const std::vector<GroundSegment>& Ground() const { return ground_; }
    render::TextureId GroundParticleTexture(const std::string& material) const {
        const auto found = groundParticleTextures_.find(material);
        return found == groundParticleTextures_.end() ? render::InvalidTexture : found->second;
    }
    void SetPlayerFootstepContact(size_t segment, float x, float y);
    void EmitBombExplosion(render::Renderer& renderer,float x,float y);
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
    const std::vector<BreakableBody>& Breakables() const { return breakables_; }
    bool DamageBreakable(size_t index,float damage);
    void ApplyBrokenBreakables(const std::unordered_set<std::string>& ids);
    std::vector<std::string> BrokenBreakableIds() const;

    size_t UpdateSpawnedOrbs(float playerX, float playerY, float dt);
    size_t PendingSpawnedOrbs() const { return spawnedOrbs_.size(); }
    void ApplySmashable(bool smashed);
    const std::vector<DojoScroll>& Scrolls() const { return scrolls_; }
    void ApplyDeflection(bool unlocked);
    void ApplyTechniques(const std::unordered_set<std::string>& owned, int completions);
    void ApplyKeys(const std::unordered_set<std::string>& keys);
    void ApplyPapers(const std::unordered_set<std::string>& papers);
    void ApplyHealthDevil(bool rewarded);
private:
    std::uint64_t sceneEpoch_ = 0;
    std::uint64_t nextObjectIdentity_ = 1;
    std::vector<SceneSprite> sprites_;
    // program.cpp stores this render's modified delta after Render. A scene
    // draw consumes the preceding rendered delta, even during modal pauses.
    float pendingModifiedDelta_ = 0.f;
    float previousRenderedModifiedDelta_ = 0.f;
    std::vector<ParticleSpray> particleSprays_;
    std::vector<ParticleSpray> healthDevilSprays_;
    std::vector<ParticleSpray> footstepSprays_;
    std::vector<ParticleSpray> jumpTakeoffSprays_;
    std::vector<ParticleSpray> landingSprays_;
    std::vector<ParticleSpray> introSwordPullSprays_;
    render::TextureId introBloodTextures_[2] = {render::InvalidTexture, render::InvalidTexture};
    std::uint32_t introBloodRandomState_ = 0x424c4f44u;
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
    float playerX_ = 0, playerY_ = 0, playerEntryDirection_ = 0;
    struct PlatformBody { AuthoredPlatformMotion motion; size_t boxIndex; bool pathing; };
    std::vector<PlatformBody> platforms_;
    struct AmbientBody { AuthoredAmbientActor motion; std::unordered_map<std::string,std::vector<render::TextureId>> frames; std::string sequence; };
    std::vector<AmbientBody> ambientActors_;
    double dynamicsClock_ = 0;
    std::vector<ForceRegion> forceRegions_;
    std::vector<BreakableBody> breakables_;
    std::vector<HealingSpring> healingSprings_;
    std::vector<Instrument> instruments_;
    std::vector<std::string> sceneSounds_;
    float playerVelocityX_=0,playerVelocityY_=0;
    float playerVisualWidth_ = 0, playerVisualHeight_ = 0;
    struct PlayerSequence { std::vector<render::TextureId> frames; std::vector<bool> footstepFrames; float rate = 0, minRate = 0, maxRate = 60, currentRate = 0; };
    void EmitRunningFootstep(const PlayerSequence& sequence, int movementDirection);
    std::unordered_map<std::string, PlayerSequence> playerSequences_;
    std::unordered_map<std::string, PlayerSequence> playerArmSequences_;
    std::string playerSequence_;
    IntroBodyPhase introBodyPhase_ = IntroBodyPhase::None;
    bool introBodyFinished_ = false;
    std::string playerArmSequence_;
    size_t playerFrame_ = 0;
    size_t lastRunningFrame_ = static_cast<size_t>(-1); // Shared numeric guard for horizontal Running and strafe frames.
    size_t playerArmFrame_ = 0; // sequence length means finished, matching AnimationPart::IsFinished
    float playerFrameElapsed_ = 0;
    bool impactVisualFinished_ = false;
    // Character stamps the first dry descending update; off-solid time can bypass the hold.
    double playerOffSolidTime_ = 0, playerDescentTime_ = 0;
    bool playerDescending_ = false;
    float playerArmFrameElapsed_ = 0;
    int playerFacing_ = 1;
    int playerAimLineFacing_ = 1;
    float playerAimLineRotation_ = 0.f; // Authored radians, before SDL reflection conversion.
    bool playerAimLineFlipY_ = false, playerAimLineDirected_ = false;
    bool playerGrounded_ = false;
    bool playerCrouching_ = false, playerWasCrouching_ = false;
    float aimLineHold_ = 0.f, aimLineAlpha_ = 0.f;
    std::string levelPath_;
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
