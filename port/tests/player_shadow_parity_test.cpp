// Runtime Character shadow: ground depth, attachment, and solid-surface gating.
#include "../game/LevelScene.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

class CaptureRenderer final : public deathleak::render::Renderer {
public:
    std::unordered_map<std::string, deathleak::render::TextureId> textures;
    std::vector<deathleak::render::Sprite> drawn;
    bool Initialize(const char*, int, int, std::string&) override { return true; }
    deathleak::render::TextureId LoadTexture(const std::string& path, std::string&) override {
        return textures.try_emplace(path, static_cast<deathleak::render::TextureId>(textures.size() + 1)).first->second;
    }
    void BeginFrame(deathleak::render::Color) override {}
    void DrawSprite(const deathleak::render::Sprite& sprite) override { drawn.push_back(sprite); }
    void FillRect(deathleak::render::Rect, deathleak::render::Color) override {}
    void ApplyScenePostprocess() override {}
    void EndFrame() override {}
    bool PollQuit() override { return false; }
    bool KeyDown(int) const override { return false; }
    deathleak::render::MouseState Mouse() const override { return {}; }
    bool SaveFrame(const std::string&, std::string&) override { return false; }
};

int main() {
    CaptureRenderer renderer;
    deathleak::game::LevelScene scene;
    std::string error;
    assert(scene.Load("Game/XmlFiles/levels/forest_exploration_1.xml", renderer, error));
    assert(scene.SpriteCount() == 71); // Runtime child is not an authored XML sprite.
    const auto shadow = renderer.textures.at("Game/Media/characters/player/shadow.png");
    auto countShadow = [&]() {
        int count = 0;
        for (const auto& sprite : renderer.drawn) count += sprite.texture == shadow;
        return count;
    };
    const float x = scene.PlayerX(), y = scene.PlayerY();
    auto update = [&](bool grounded, int facing) {
        scene.UpdatePlayerVisual(1.f/60.f, 0, grounded ? 0 : 100, grounded, 0,
                                 false, false, false, 0, facing, false);
    };
    auto draw = [&](float px, float py) {
        renderer.drawn.clear();
        scene.Draw(renderer, px, py, px, py);
    };
    update(true, 1);
    draw(x, y);
    assert(countShadow() == 1);
    size_t shadowIndex = 0, bodyIndex = renderer.drawn.size();
    for (size_t i = 0; i < renderer.drawn.size(); ++i) {
        if (renderer.drawn[i].texture == shadow) shadowIndex = i;
        if (renderer.drawn[i].texture == renderer.textures.at("Game/Media/characters/player/idle_default/idle_default_000.png")) bodyIndex = i;
    }
    assert(shadowIndex < bodyIndex); // kGround precedes kPlayer.
    const auto first = renderer.drawn[shadowIndex];
    assert(std::abs(first.destination.x - (640.f - 100.f*2.f/3.f)) < .01f);
    assert(std::abs(first.destination.y - (360.f + (150.f-64.f)*2.f/3.f)) < .01f);
    assert(std::abs(first.destination.width - 200.f*2.f/3.f) < .01f);
    assert(std::abs(first.destination.height - 128.f*2.f/3.f) < .01f);
    assert(first.tint.a == 255 && !first.flipX && first.flipY);
    update(true, -1);
    draw(x + 40, y + 30);
    assert(countShadow() == 1);
    const auto moved = renderer.drawn[shadowIndex];
    assert(std::abs(moved.destination.x - first.destination.x) < .01f); // Camera follows player.
    assert(std::abs(moved.destination.y - first.destination.y) < .01f);
    assert(!moved.flipX); // Attachment ignores parent orientation.
    update(false, -1);
    draw(x + 40, y + 30);
    assert(countShadow() == 0);
    update(true, -1);
    draw(x + 40, y + 30);
    assert(countShadow() == 1);
    std::cout << "PASS: grounded player shadow depth, 200x128 offset, orientation, airborne suppression\n";
}
