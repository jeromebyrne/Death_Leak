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
    setenv("DEATHLEAK_RENDER_ONLY_XML_IDS", "119,122", 1);
    CaptureRenderer renderer;
    deathleak::game::LevelScene scene;
    std::string error;
    assert(scene.Load("Game/XmlFiles/levels/forest_exploration_1.xml", renderer, error));
    auto frame = [&]() {
        renderer.drawn.clear();
        scene.Draw(renderer, 0.f, 3400.f, 0.f, 3400.f);
        assert(renderer.drawn.size() == 2);
        return renderer.drawn;
    };
    auto xFor = [](float worldX) { return 640.f + (worldX - 994.f) * (2.f/3.f); };
    auto near = [](float a, float b) { assert(std::abs(a-b) < .02f); };
    const auto initial = frame();
    assert(initial[0].tint.a == 128 && initial[1].tint.a == 153);
    for (const auto& fog : initial) {
        assert(fog.pixelWobble.enabled);
        near(fog.pixelWobble.intensity, .07f);
        near(fog.pixelWobble.speedMod, .3f);
        near(fog.pixelWobble.time, 0.f);
    }
    assert(!initial[0].flipY && initial[1].flipY);
    near(initial[0].destination.x, xFor(789.985352f));
    near(initial[1].destination.x, xFor(-125.014648f));

    // Both XML IDs fade independently; the movement starts from StartLerpX.
    scene.UpdateVisuals(1.f, 0.f, 3400.f, 0.f, 3400.f);
    const auto one = frame();
    near(one[0].pixelWobble.time, 1.f);
    near(one[1].pixelWobble.time, 1.f);
    assert(one[0].tint.a == 21 && one[1].tint.a == 26);
    near(one[0].destination.x, xFor(789.985352f + 1000.f/45.f));
    near(one[1].destination.x, xFor(-125.014648f - 750.f/70.f));
    for (int i = 0; i < 4; ++i) scene.UpdateVisuals(1.f, 0.f, 3400.f, 0.f, 3400.f);
    const auto five = frame();
    assert(five[0].tint.a == 106 && five[1].tint.a == 128);
    scene.UpdateVisuals(1.f, 0.f, 3400.f, 0.f, 3400.f);
    const auto six = frame();
    near(six[0].pixelWobble.time, 6.f);
    assert(six[0].tint.a == 128 && six[1].tint.a == 153);

    // Off-zone objects keep elapsed, position, and alpha unchanged.
    scene.UpdateVisuals(1.f, 9000.f, 9000.f, 9000.f, 9000.f);
    const auto paused = frame();
    near(paused[0].pixelWobble.time, 7.f); // independent of update zone
    near(paused[0].destination.x, six[0].destination.x);
    assert(paused[0].tint.a == six[0].tint.a);

    for (int i = 0; i < 38; ++i) scene.UpdateVisuals(1.f, 0.f, 3400.f, 0.f, 3400.f);
    const auto beforeReset = frame(); // elapsed 44: fade-out is active.
    assert(beforeReset[0].tint.a == 21);
    scene.UpdateVisuals(1.f, 0.f, 3400.f, 0.f, 3400.f);
    const auto skippedEndpoint = frame(); // elapsed 45 resets, but skips drawing update.
    near(skippedEndpoint[0].destination.x, beforeReset[0].destination.x);
    assert(skippedEndpoint[0].tint.a == beforeReset[0].tint.a);
    scene.UpdateVisuals(1.f, 0.f, 3400.f, 0.f, 3400.f);
    const auto restarted = frame();
    near(restarted[0].destination.x, one[0].destination.x);
    assert(restarted[0].tint.a == one[0].tint.a);
    // Strict source thresholds: 10 remains forward, 11 reverses, then 4
    // reverses back. Reload also resets the scene-local clock.
    assert(scene.Load("Game/XmlFiles/levels/forest_exploration_1.xml", renderer, error));
    near(frame()[0].pixelWobble.time, 0.f);
    for (int i = 0; i < 10; ++i) scene.UpdateVisuals(1.f, 0.f, 3400.f, 0.f, 3400.f);
    near(frame()[0].pixelWobble.time, 10.f);
    scene.UpdateVisuals(1.f, 0.f, 3400.f, 0.f, 3400.f);
    near(frame()[0].pixelWobble.time, 11.f);
    scene.UpdateVisuals(1.f, 0.f, 3400.f, 0.f, 3400.f);
    near(frame()[0].pixelWobble.time, 10.f);
    for (int i = 0; i < 6; ++i) scene.UpdateVisuals(1.f, 0.f, 3400.f, 0.f, 3400.f);
    near(frame()[0].pixelWobble.time, 4.f);
    scene.UpdateVisuals(1.f, 0.f, 3400.f, 0.f, 3400.f);
    near(frame()[0].pixelWobble.time, 5.f);
    std::cout << "PASS: forest ScrollingSprite 119/122 movement, fade, zone, endpoint\n";
}
