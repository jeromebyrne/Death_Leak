// Source Player's runtime 400x400 aim-line child, including pre-cooldown refresh.
#include "../game/LevelScene.h"
#include <cassert>
#include <cmath>
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
        return textures.try_emplace(path, static_cast<deathleak::render::TextureId>(textures.size()+1)).first->second;
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
    setenv("DEATHLEAK_RENDER_ONLY_XML_IDS", "2,__player_aim_line", 1);
    CaptureRenderer renderer;
    deathleak::game::LevelScene scene;
    std::string error;
    assert(scene.Load("Game/XmlFiles/levels/forest_exploration_1.xml", renderer, error));
    assert(scene.SpriteCount() == 71);
    const auto aim = renderer.textures.at("Game/Media/aim_line.png");
    const float x = scene.PlayerX(), y = scene.PlayerY();
    constexpr float dt = 1.f/60.f, scale = 2.f/3.f;
    auto update = [&](int facing, bool attempt, bool shot = false) {
        scene.UpdatePlayerVisual(dt, 0, 0, true, 0, false, false, false, 0, facing, shot, attempt);
    };
    auto findAim = [&]() -> const deathleak::render::Sprite* {
        renderer.drawn.clear();
        scene.Draw(renderer, x, y, x, y);
        for (const auto& sprite : renderer.drawn) if (sprite.texture == aim) return &sprite;
        return nullptr;
    };
    assert(!findAim());
    update(1, true, true);
    auto* line = findAim();
    assert(line && line->tint.a == 255);
    assert(std::abs(line->destination.x - (640.f+(5.f-200.f)*scale)) < .01f);
    assert(std::abs(line->destination.y - (360.f+(-25.f-200.f)*scale)) < .01f);
    assert(std::abs(line->destination.width - 400.f*scale) < .01f);
    assert(!line->flipX && line->flipY);
    assert(renderer.drawn.back().texture == aim); // kPlayer child after body.
    update(-1, true, false); // Eligible attempt during cooldown: no projectile.
    line = findAim();
    assert(line && line->tint.a == 255 && !line->flipX && !line->flipY);
    assert(std::abs(line->destination.x - (640.f+(-5.f-200.f)*scale)) < .01f);
    for (int i=0; i<3; ++i) update(-1, false);
    line = findAim();
    assert(line && line->tint.a < 255 && line->tint.a > 0); // 10/s fade.
    for (int i=0; i<7; ++i) update(-1, false);
    assert(!findAim());
    update(-1, true, false);
    assert(findAim()); // Fresh eligible attempt restores alpha.
    std::cout << "PASS: source aim-line attachment, depth, facing, cooldown refresh and fade\n";
}
