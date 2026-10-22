#include "../game/LevelScene.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

class CaptureRenderer final : public deathleak::render::Renderer {
public:
    std::vector<std::string> loaded;
    std::vector<deathleak::render::Sprite> drawn;
    bool Initialize(const char*, int, int, std::string&) override { return true; }
    deathleak::render::TextureId LoadTexture(const std::string& path, std::string&) override {
        loaded.push_back(path); return static_cast<deathleak::render::TextureId>(loaded.size());
    }
    void BeginFrame(deathleak::render::Color) override {}
    void DrawSprite(const deathleak::render::Sprite& s) override { drawn.push_back(s); }
    void FillRect(deathleak::render::Rect, deathleak::render::Color) override {}
    void ApplyScenePostprocess() override {}
    void EndFrame() override {}
    bool PollQuit() override { return false; }
    bool KeyDown(int) const override { return false; }
    deathleak::render::MouseState Mouse() const override { return {}; }
    bool SaveFrame(const std::string&, std::string&) override { return false; }
};
int main() {
    setenv("DEATHLEAK_RENDER_ONLY_XML_IDS", "49", 1);
    CaptureRenderer renderer;
    deathleak::game::LevelScene scene;
    std::string error;
    assert(scene.Load("Game/XmlFiles/levels/waterfall_top.xml", renderer, error));
    assert(std::count(renderer.loaded.begin(), renderer.loaded.end(), "Game/Media/waterfall_splash.png") == 1);
    scene.UpdateVisuals(1.f/60.f, 321.f, 3530.f, 321.f, 3530.f);
    scene.Draw(renderer, 321.f, 3530.f, 321.f, 3530.f);
    assert(!renderer.drawn.empty());
    assert(renderer.drawn.size() <= 100);
    const auto splashTexture = static_cast<deathleak::render::TextureId>(
        1 + std::find(renderer.loaded.begin(), renderer.loaded.end(), "Game/Media/waterfall_splash.png") - renderer.loaded.begin());
    for (const auto& sprite : renderer.drawn) {
        assert(sprite.texture == splashTexture); // Filter isolates ID49 despite other admitted sprays.
        assert(sprite.destination.width >= 250.f*2.f/3.f);
        assert(sprite.destination.width <= 450.f*2.f/3.f);
        assert(sprite.destination.width == sprite.destination.height);
    }
    // Offscreen object still may update in the broad source zone, but should
    // contribute no onscreen particles in this distant camera view.
    renderer.drawn.clear();
    scene.Draw(renderer, 8000.f, 8000.f, 8000.f, 8000.f);
    assert(renderer.drawn.empty());
    CaptureRenderer forestRenderer;
    deathleak::game::LevelScene forest;
    assert(forest.Load("Game/XmlFiles/levels/forest_exploration_1.xml", forestRenderer, error));
    assert(std::count(forestRenderer.loaded.begin(), forestRenderer.loaded.end(),
        "Game/Media/levels/forest_exploration_1/oak_leaf.png") == 1);
    assert(std::count(forestRenderer.loaded.begin(), forestRenderer.loaded.end(),
        "Game/Media/waterfall_splash.png") == 0);
    std::cout << "PASS: ID49 textured quads, ID49 isolated, distant view clear; forest ID47 retained\n";
}
