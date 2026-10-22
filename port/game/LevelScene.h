#pragma once
#include "../render/Renderer.h"
#include "../audio/SoundPlayer.h"
#include "ParticleSpray.h"
#include <cstddef>
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
    bool parallaxLayer = false, followCameraX = false, followCameraY = false;
    float cameraParallaxX = 0, cameraParallaxY = 0;
    bool currencyOrb = false;
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

struct GroundSegment { float x1, y1, x2, y2; };
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

class LevelScene {
public:
    bool Load(const std::string& path, render::Renderer& renderer, std::string& error,
              const std::string& fromLevel = {}, const std::string& entryDoorId = {});
    void Draw(render::Renderer& renderer, float cameraX, float cameraY, float playerX, float playerY) const;
    void UpdatePlayerVisual(float dt, float vx, float vy, bool grounded, int wallSide,
                            bool rolling, bool dashing, bool inWater, float meleeElapsed, int facing,
                            bool projectileFired);
    void UpdateVisuals(float dt, float playerX, float playerY, float cameraX, float cameraY);
    size_t CollectCurrencyOrbs(float playerX, float playerY, std::unordered_set<std::string>& collected);
    void ApplyCollected(const std::unordered_set<std::string>& collected);
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
    size_t SpriteCount() const { return sprites_.size(); }
    size_t SkippedCount() const { return skipped_; }
    float PlayerX() const { return playerX_; }
    float PlayerY() const { return playerY_; }
    float PlayerVisualWidth() const { return playerVisualWidth_; }
    float PlayerVisualHeight() const { return playerVisualHeight_; }
    const std::vector<GroundSegment>& Ground() const { return ground_; }
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
    struct PlayerSequence { std::vector<render::TextureId> frames; float rate = 0, minRate = 0, maxRate = 60; };
    std::unordered_map<std::string, PlayerSequence> playerSequences_;
    std::unordered_map<std::string, PlayerSequence> playerArmSequences_;
    std::string playerSequence_;
    std::string playerArmSequence_;
    size_t playerFrame_ = 0;
    size_t playerArmFrame_ = 0; // sequence length means finished, matching AnimationPart::IsFinished
    float playerFrameElapsed_ = 0;
    float playerArmFrameElapsed_ = 0;
    int playerFacing_ = 1;
    float cameraX_ = 0, cameraY_ = 0;
    float cameraOffsetX_ = 0, cameraOffsetY_ = 0;
    float cameraLagX_ = 7.5f, cameraLagY_ = 15.f, cameraZoom_ = 1.f;
    bool cameraFollowX_ = true, cameraFollowY_ = true;
    bool cameraBounds_ = false;
    float cameraBoundLeft_ = 0, cameraBoundRight_ = 0, cameraBoundTop_ = 0, cameraBoundBottom_ = 0;
    float visualTime_ = 0;
    float pixelWobbleTime_ = 0;
    bool pixelWobbleReverse_ = false;
    size_t skipped_ = 0;
};
}
