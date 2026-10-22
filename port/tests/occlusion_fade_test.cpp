// Deterministic authored-forest fade test; build/run from source/.
#include "../game/LevelScene.h"
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>

class CaptureRenderer final : public deathleak::render::Renderer {
public:
    std::vector<deathleak::render::Sprite> drawn;
    bool Initialize(const char*, int, int, std::string&) override { return true; }
    deathleak::render::TextureId LoadTexture(const std::string&, std::string&) override { return 1; }
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
    setenv("DEATHLEAK_RENDER_ONLY_XML_IDS", "5,6", 1);
    CaptureRenderer renderer;
    deathleak::game::LevelScene scene;
    std::string error;
    assert(scene.Load("Game/XmlFiles/levels/forest_exploration_1.xml", renderer, error));
    auto alphaAt = [&](float x, float y) {
        renderer.drawn.clear();
        scene.Draw(renderer, x, y, x, y);
        assert(renderer.drawn.size() == 1); // The other authored tree is offscreen.
        return renderer.drawn.front().tint.a;
    };
    constexpr float dt = 1.f / 60.f;
    constexpr float x5 = -1174.014648f, y5 = 4065.f;
    constexpr float x6 = 1339.985352f, y6 = 3895.f;
    assert(alphaAt(x5, y5) == 255 && alphaAt(x6, y6) == 255);
    scene.UpdateVisuals(dt, x6, y6, x6, y6);
    assert(alphaAt(x6, y6) == 249); // 1 - 1.5/60, quantized to 8-bit.
    assert(alphaAt(x5, y5) == 255);
    for (int i = 0; i < 24; ++i) scene.UpdateVisuals(dt, x6, y6, x6, y6);
    assert(alphaAt(x6, y6) == 96); // Source's one-tick overshoot to 0.375.
    scene.UpdateVisuals(dt, x6, y6, x6, y6);
    assert(alphaAt(x6, y6) == 102); // Settles at authored 0.4.
    scene.UpdateVisuals(dt, x5, y5, x5, y5);
    assert(alphaAt(x5, y5) == 249); // ID 5 independently opts in.
    assert(alphaAt(x6, y6) == 108); // ID 6 starts restoring when outside.
    for (int i = 0; i < 24; ++i) scene.UpdateVisuals(dt, x5, y5, x5, y5);
    assert(alphaAt(x6, y6) == 255);
    // Outside the manager update zone, the prior visual alpha is retained.
    const auto before = alphaAt(x5, y5);
    scene.UpdateVisuals(dt, 9000.f, 9000.f, 9000.f, 9000.f);
    assert(alphaAt(x5, y5) == before);
    std::cout << "PASS: authored forest tree 5/6 fade, restore, endpoint, update-zone\n";
}
