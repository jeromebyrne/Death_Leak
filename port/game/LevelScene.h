#pragma once
#include "../render/Renderer.h"
#include <string>
#include <vector>

namespace deathleak::game {
struct SceneSprite {
    render::TextureId texture = render::InvalidTexture;
    float x = 0, y = 0, width = 0, height = 0;
    float rotation = 0;
    int depth = 0;
    render::Color tint{255, 255, 255, 255};
    bool flipX = false, flipY = false;
};

class LevelScene {
public:
    bool Load(const std::string& path, render::Renderer& renderer, std::string& error);
    void Draw(render::Renderer& renderer, float cameraX, float cameraY) const;
    float CameraX() const { return cameraX_; }
    float CameraY() const { return cameraY_; }
    size_t SpriteCount() const { return sprites_.size(); }
    size_t SkippedCount() const { return skipped_; }
private:
    std::vector<SceneSprite> sprites_;
    float cameraX_ = 0, cameraY_ = 0;
    size_t skipped_ = 0;
};
}
