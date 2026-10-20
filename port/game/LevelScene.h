#pragma once
#include "../render/Renderer.h"
#include <string>
#include <unordered_set>
#include <vector>

namespace deathleak::game {
struct SceneSprite {
    render::TextureId texture = render::InvalidTexture;
    float x = 0, y = 0, width = 0, height = 0;
    float rotation = 0;
    int depth = 0;
    render::Color tint{255, 255, 255, 255};
    bool flipX = false, flipY = false;
    bool player = false;
    bool currencyOrb = false;
    bool keyPickup = false;
    bool dojoScroll = false;
    bool collected = false;
    std::string persistentId;
};

struct GroundSegment { float x1, y1, x2, y2; };
struct SolidBox { float left, right, bottom, top; float damage = 0; };
struct LevelTrigger { float x, y, width, height; std::string destination; };
struct EncounterTrigger { float x, y, width, height; int count; std::string type; std::string id; };
struct Door { float x, y, width, height; std::string destination, requiredKey, id; };
struct Boat { float x, y, width, height; std::string id; };
struct DojoScroll { float x, y, width, height; std::string feature; std::string id; };
struct KeyPickup { float x, y, width, height; std::string keyId; };
struct HazardRegion { float x, y, width, height; bool water; float damage; };

class LevelScene {
public:
    bool Load(const std::string& path, render::Renderer& renderer, std::string& error,
              const std::string& fromLevel = {}, const std::string& entryDoorId = {});
    void Draw(render::Renderer& renderer, float cameraX, float cameraY, float playerX, float playerY) const;
    size_t CollectCurrencyOrbs(float playerX, float playerY, std::unordered_set<std::string>& collected);
    void ApplyCollected(const std::unordered_set<std::string>& collected);
    float CameraX() const { return cameraX_; }
    float CameraY() const { return cameraY_; }
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
    const std::vector<Door>& Doors() const { return doors_; }
    const std::vector<Boat>& Boats() const { return boats_; }
    const std::vector<KeyPickup>& Keys() const { return keys_; }
    const std::vector<DojoScroll>& Scrolls() const { return scrolls_; }
    void ApplyDeflection(bool unlocked);
    void ApplyKeys(const std::unordered_set<std::string>& keys);
private:
    std::vector<SceneSprite> sprites_;
    std::vector<GroundSegment> ground_;
    std::vector<SolidBox> solidBoxes_;
    std::vector<LevelTrigger> triggers_;
    std::vector<EncounterTrigger> encounters_;
    std::vector<HazardRegion> hazards_;
    std::vector<Door> doors_;
    std::vector<Boat> boats_;
    std::vector<KeyPickup> keys_;
    std::vector<DojoScroll> scrolls_;
    float playerX_ = 0, playerY_ = 0;
    float playerVisualWidth_ = 0, playerVisualHeight_ = 0;
    float cameraX_ = 0, cameraY_ = 0;
    size_t skipped_ = 0;
};
}
