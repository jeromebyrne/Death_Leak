// Grounded crouch/release command-path texture oracle for Forest-1 Player ID2.
#include "../game/LevelScene.h"
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

class SpyRenderer final : public deathleak::render::Renderer {
public:
    std::unordered_map<deathleak::render::TextureId, std::string> paths;
    std::vector<deathleak::render::Sprite> drawn;
    bool Initialize(const char*, int, int, std::string&) override { return true; }
    deathleak::render::TextureId LoadTexture(const std::string& path, std::string&) override {
        auto id = ++next; paths[id] = path; return id;
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
private:
    deathleak::render::TextureId next = 0;
};

int main() {
    setenv("DEATHLEAK_RENDER_ONLY_XML_IDS", "2", 1);
    SpyRenderer renderer;
    deathleak::game::LevelScene scene;
    std::string error;
    assert(scene.Load("Game/XmlFiles/levels/forest_exploration_1.xml", renderer, error));
    const auto x = scene.PlayerX(), y = scene.PlayerY();
    const auto body = [&]() {
        renderer.drawn.clear(); scene.Draw(renderer, x, y, x, y);
        for (const auto& sprite : renderer.drawn) {
            const auto& path = renderer.paths.at(sprite.texture);
            if (path.find("Game/Media/characters/player/back_arm_") != 0) return path;
        }
        assert(false); return std::string{};
    };
    const auto path = [](int frame) {
        return "Game/Media/characters/player/duck_default/duck_default_00" + std::to_string(frame) + ".png";
    };
    const auto tick = [&](bool crouch, float prior, float vx = 0, bool grounded = true, bool rolling = false) {
        scene.UpdatePlayerVisual(1.f/60.f, vx, 0, grounded, 0, rolling, false, false, 0, 1,
                                 false, false, {}, false, 0, 0, prior, crouch);
        return body();
    };
    assert(tick(true, 0.f, 120.f) == path(0)); // Crouch outranks residual running velocity.
    float elapsed = 0;
    int frame = 0;
    for (int i = 0; i < 24; ++i) {
        elapsed += 1.f/60.f;
        if (elapsed >= 1.f/40.f && frame < 6) { elapsed -= 1.f/40.f; ++frame; }
        assert(tick(true, 1.f/60.f, 120.f) == path(std::min(frame, 5)));
    }
    // Release starts the authored reverse sequence at its first frame and
    // advances at most once per update using the preceding rendered delta.
    assert(tick(false, 0.f) == path(5));
    elapsed = 0; frame = 0;
    for (int i = 0; i < 9; ++i) {
        elapsed += 1.f/60.f;
        if (elapsed >= 1.f/40.f && frame < 6) { elapsed -= 1.f/40.f; ++frame; }
        assert(tick(false, 1.f/60.f) == path(5 - std::min(frame, 5)));
    }
    assert(tick(false, 1.f/60.f).find("idle_default") != std::string::npos);
    assert(tick(true, 0.f) == path(0));
    assert(tick(false, 0.f, 100.f).find("run_default") != std::string::npos);
    assert(tick(false, 0.f).find("idle_default") != std::string::npos); // Running canceled CrouchUp.
    assert(tick(true, 0.f) == path(0));
    assert(tick(false, 0.f, 0, true, true).find("roll_default") != std::string::npos);
    assert(tick(false, 0.f).find("idle_default") != std::string::npos); // Roll canceled CrouchUp.
    std::cout << "PASS: authored crouch/stand frame order, 40-fps clock, final hold, priority, interruption\n";
}
