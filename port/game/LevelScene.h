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
    bool collected = false;
    std::string persistentId;
};

struct GroundSegment { float x1, y1, x2, y2; };
struct LevelTrigger { float x, y, width, height; std::string destination; };

class LevelScene {
public:
    bool Load(const std::string& path, render::Renderer& renderer, std::string& error,
              const std::string& fromLevel = {});
    void Draw(render::Renderer& renderer, float cameraX, float cameraY, float playerX, float playerY) const;
    size_t CollectCurrencyOrbs(float playerX, float playerY, std::unordered_set<std::string>& collected);
    void ApplyCollected(const std::unordered_set<std::string>& collected);
    float CameraX() const { return cameraX_; }
    float CameraY() const { return cameraY_; }
    size_t SpriteCount() const { return sprites_.size(); }
    size_t SkippedCount() const { return skipped_; }
    float PlayerX() const { return playerX_; }
    float PlayerY() const { return playerY_; }
    const std::vector<GroundSegment>& Ground() const { return ground_; }
    const std::vector<LevelTrigger>& Triggers() const { return triggers_; }
private:
    std::vector<SceneSprite> sprites_;
    std::vector<GroundSegment> ground_;
    std::vector<LevelTrigger> triggers_;
    float playerX_ = 0, playerY_ = 0;
    float cameraX_ = 0, cameraY_ = 0;
    size_t skipped_ = 0;
};
}
