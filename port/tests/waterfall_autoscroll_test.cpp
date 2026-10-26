#include "../game/LevelScene.h"
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>

using namespace deathleak;
class CaptureRenderer final : public render::Renderer {
public:
    std::vector<render::Sprite> drawn;
    bool Initialize(const char*, int, int, bool, std::string&) override { return true; }
    render::TextureId LoadTexture(const std::string&, std::string&) override { return 1; }
    void BeginFrame(render::Color) override {}
    void DrawSprite(const render::Sprite& s) override { drawn.push_back(s); }
    void FillRect(render::Rect, render::Color) override {}
    void ApplyScenePostprocess() override {}
    void EndFrame() override {}
    bool PollQuit() override { return false; }
    bool KeyDown(int) const override { return false; }
    render::MouseState Mouse() const override { return {}; }
    bool SaveFrame(const std::string&, std::string&) override { return false; }
};
static void near(float actual, float expected) { assert(std::fabs(actual - expected) < 1e-6f); }
int main() {
    setenv("DEATHLEAK_RENDER_ONLY_XML_IDS", "6,13", 1);
    CaptureRenderer renderer;
    game::LevelScene scene;
    std::string error;
    assert(scene.Load("Game/XmlFiles/levels/waterfall_top.xml", renderer, error));
    auto update = [&](float dt, bool paused = false) {
        scene.UpdateVisuals(dt, scene.PlayerX(), scene.PlayerY(), scene.CameraX(), scene.CameraY(), paused);
    };
    auto draw = [&](float cameraY = 3000.f, bool fresh = true) {
        if (fresh) scene.PreparePresentation();
        renderer.drawn.clear();
        scene.Draw(renderer, scene.CameraX(), cameraY, scene.PlayerX(), scene.PlayerY());
        assert(renderer.drawn.size() == 2);
        return renderer.drawn;
    };
    constexpr float step6 = 7.f * (720.f / 1080.f) * .001f;
    constexpr float step13 = 11.f * (720.f / 1080.f) * .001f;
    // Draw 1 sees Timing::LastUpdateDelta's initial zero, despite an update.
    update(1.f / 60.f);
    auto first = draw(); near(first[0].uvShiftY, 0.f); near(first[1].uvShiftY, 0.f);
    update(1.f / 60.f);
    auto second = draw(); near(second[0].uvShiftY, step6); near(second[1].uvShiftY, step13);
    // Modal object pause does not pause the ParallaxLayer draw clock.
    update(1.f / 120.f, true);
    auto third = draw(); near(third[0].uvShiftY, 2 * step6); near(third[1].uvShiftY, 2 * step13);
    update(1.f / 60.f);
    auto fourth = draw(); near(fourth[0].uvShiftY, 2.5f * step6); near(fourth[1].uvShiftY, 2.5f * step13);
    // No scene draw between updates: no hidden UV advances, and the most
    // recent modified update delta is stored only after the next actual draw.
    update(1.f / 120.f); update(1.f / 60.f);
    auto fifth = draw(); near(fifth[0].uvShiftY, 3.5f * step6); near(fifth[1].uvShiftY, 3.5f * step13);
    // A refresh-only repeated draw does not advance V or prior-render state.
    auto sixth = draw(3000.f, false); near(sixth[0].uvShiftY, 3.5f * step6); near(sixth[1].uvShiftY, 3.5f * step13);
    for (int repeat = 0; repeat < 100000; ++repeat) {
        auto same = draw(3000.f, false);
        for (size_t i = 0; i < 2; ++i) {
            near(same[i].uvShiftY, fifth[i].uvShiftY);
            assert(same[i].texture == fifth[i].texture);
            assert(same[i].destination.x == fifth[i].destination.x);
            assert(same[i].destination.y == fifth[i].destination.y);
        }
    }
    for (size_t i = 0; i < 2; ++i) {
        const auto& a = first[i]; const auto& b = sixth[i];
        assert(a.destination.x == b.destination.x && a.destination.y == b.destination.y);
        assert(a.opacity == b.opacity && a.flipX == b.flipX && a.flipY == b.flipY);
        assert(a.pixelWobble.enabled == b.pixelWobble.enabled);
    }
    // Loading another authored scene resets per-object V but preserves the
    // process-owned previous render delta on this scene instance.
    assert(scene.Load("Game/XmlFiles/levels/waterfall_top.xml", renderer, error));
    update(1.f / 60.f);
    auto reset = draw(); near(reset[0].uvShiftY, step6); near(reset[1].uvShiftY, step13);
    std::cout << "PASS: waterfall IDs 6/13 prior-render modified-delta cadence, modal, repeat-state, reset\n";
}
