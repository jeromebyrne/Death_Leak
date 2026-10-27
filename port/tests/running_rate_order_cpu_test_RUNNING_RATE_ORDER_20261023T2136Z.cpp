// CPU-only live Forest-1 ID2 body draw-command regression for Running rate order.
#include "../game/LevelScene.h"
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace render = deathleak::render;
struct PathSpy final : render::Renderer {
    std::unordered_map<std::string, render::TextureId> ids;
    std::unordered_map<render::TextureId, std::string> paths;
    std::vector<std::string> drawn;
    bool Initialize(const char*, int, int, bool, std::string&) override { return true; }
    render::TextureId LoadTexture(const std::string& path, std::string&) override {
        auto [it, inserted] = ids.try_emplace(path, static_cast<render::TextureId>(ids.size() + 1));
        if (inserted) paths.emplace(it->second, path);
        return it->second;
    }
    void BeginFrame(render::Color) override {}
    void DrawSprite(const render::Sprite& sprite) override { drawn.push_back(paths.at(sprite.texture)); }
    void FillRect(render::Rect, render::Color) override {}
    void ApplyScenePostprocess() override {}
    void EndFrame() override {}
    bool PollQuit() override { return false; }
    bool KeyDown(int) const override { return false; }
    render::MouseState Mouse() const override { return {}; }
    bool SaveFrame(const std::string&, std::string&) override { return false; }
};

int main() {
    setenv("DEATHLEAK_RENDER_ONLY_XML_IDS", "2", 1);
    PathSpy spy;
    deathleak::game::LevelScene scene;
    std::string error;
    if (!scene.Load("Game/XmlFiles/levels/forest_exploration_1.xml", spy, error)) {
        std::cerr << error << '\n'; return 1;
    }
    const std::string run = "Game/Media/characters/player/run_default/run_default_";
    assert(spy.ids.count(run + "000.png") && spy.ids.count(run + "001.png"));
    const float x = scene.PlayerX(), y = scene.PlayerY();
    auto bodyPath = [&]() {
        spy.drawn.clear();
        scene.Draw(spy, x, y, x, y);
        std::string body;
        for (const auto& path : spy.drawn) {
            if (path.find("Game/Media/characters/player/") != 0 ||
                path.find("/back_arm_") != std::string::npos) continue;
            assert(body.empty());
            body = path;
        }
        assert(!body.empty());
        return body;
    };
    // dt deliberately differs from the preceding rendered modified delta.
    auto tick = [&](float vx, float prior) {
        scene.UpdatePlayerVisual(.003f, vx, 0.f, true, 0, false, false,
                                 false, 0.f, 1, false, false, {}, false,
                                 0.f, 0.f, prior);
        return bodyPath();
    };
    assert(tick(800.f, .016f) == run + "000.png"); // XML 60 fps, carry .016.
    assert(tick(60.f, .008f) == run + "001.png");  // Stored 60 fps, then store 4.77.
    assert(tick(60.f, .100f) == run + "001.png");  // Low rate now applies; carry below period.
    assert(tick(60.f, .105f) == run + "002.png");  // Fractional carry crosses 1/4.77.
    assert(tick(60.f, .500f) == run + "003.png");  // One step maximum even with excess time.
    assert(tick(60.f, .001f) == run + "004.png");  // Excess carry remains for next tick.

    // Leave Running. Re-entry resets frame/elapsed, not the sequence's stored 4.77 fps.
    assert(tick(0.f, .001f).find("/idle_default/") != std::string::npos);
    assert(tick(800.f, .016f) == run + "000.png"); // Would be 001 if rate reset to XML 60.
    assert(tick(800.f, .001f) == run + "001.png"); // Prior tick stored 60; .017 crosses period.
    std::cout << "PASS: live Forest-1 ID2 Running stored-rate advance, speed transition, carry, and re-entry texture paths\n";
}
