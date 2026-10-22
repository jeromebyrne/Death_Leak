// Source-authored Player ID2 arm composition, finite timing, and draw order.
#include "../game/LevelScene.h"
#include <cassert>
#include <cstdlib>
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
        auto [it, inserted] = textures.try_emplace(path, static_cast<deathleak::render::TextureId>(textures.size() + 1));
        return it->second;
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
    setenv("DEATHLEAK_RENDER_ONLY_XML_IDS", "2", 1);
    CaptureRenderer renderer;
    deathleak::game::LevelScene scene;
    std::string error;
    assert(scene.Load("Game/XmlFiles/levels/forest_exploration_1.xml", renderer, error));
    const auto x = scene.PlayerX(), y = scene.PlayerY();
    constexpr float dt = 1.f / 60.f;
    auto update = [&](float vx, bool fired, int facing = 1) {
        scene.UpdatePlayerVisual(dt, vx, 0, true, 0, false, false, false, 0, facing, fired);
    };
    auto draw = [&]() {
        renderer.drawn.clear();
        scene.Draw(renderer, x, y, x, y);
        return renderer.drawn.size();
    };
    auto id = [&](const std::string& path) { return renderer.textures.at("Game/" + path); };
    assert(draw() == 2); // Source initialization starts a finite Still arm pass.
    for (int i = 0; i < 20; ++i) update(0, false);
    assert(draw() == 1); // No perpetual idle arm.
    update(0, true);
    assert(draw() == 2);
    assert(renderer.drawn[0].texture == id("Media/characters/player/back_arm_attack/back_arm_attack_001.png"));
    assert(renderer.drawn[0].texture != renderer.drawn[1].texture);
    assert(renderer.drawn[0].destination.x == renderer.drawn[1].destination.x);
    assert(renderer.drawn[0].destination.y == renderer.drawn[1].destination.y);
    update(0, false);
    assert(draw() == 2);
    assert(renderer.drawn[0].texture == id("Media/characters/player/back_arm_attack/back_arm_attack_001.png"));
    for (int i = 0; i < 18; ++i) update(0, false);
    assert(draw() == 1); // Frame cursor reached sequence length.
    update(0, true, -1);
    assert(draw() == 2 && renderer.drawn[0].flipX && renderer.drawn[1].flipX);
    // A body-sequence transition selects Running and suppresses the old arm.
    update(100, false);
    assert(draw() == 1);
    update(100, true);
    assert(draw() == 2);
    assert(renderer.drawn[0].texture == id("Media/characters/player/back_arm_attack_run/back_arm_attack_run_001.png"));
    for (int i = 0; i < 20; ++i) update(100, false);
    assert(draw() == 1);
    std::cout << "PASS: Player ID2 finite Still/Running firing arm, draw order, facing, suppression\n";
}
