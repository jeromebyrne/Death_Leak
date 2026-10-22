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
    bool Initialize(const char*, int, int, std::string&) override { return true; }
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
static void near(float x, float y) { assert(std::fabs(x-y) < .0001f); }
int main() {
    CaptureRenderer renderer;
    game::ShaderClock clock;
    // Menu frames precede first level; draw order consumes the prior frame delta.
    for (int i=0; i<3; ++i) { clock.Update(.25f); const auto frame=clock.Draw(); near(frame.totalTime, (i+1)*.25f); clock.RenderCompleted(.25f); }
    std::string error;
    game::LevelScene forest;
    setenv("DEATHLEAK_RENDER_ONLY_XML_IDS", "119,122,36", 1);
    assert(forest.Load("Game/XmlFiles/levels/forest_exploration_1.xml", renderer, error));
    auto drawFog = [&](game::LevelScene& scene, game::ShaderClockSnapshot frame) {
        renderer.drawn.clear(); scene.Draw(renderer, 0.f, 3400.f, 0.f, 3400.f, {}, 1.f, 1.f, frame);
        assert(renderer.drawn.size()==2 && renderer.drawn[0].pixelWobble.enabled);
        near(renderer.drawn[0].pixelWobble.time, frame.pixelWobbleTime);
    };
    auto first = clock.Draw(); drawFog(forest, first); near(first.pixelWobbleTime, .75f);
    clock.RenderCompleted(.25f);
    for (int i=0; i<4; ++i) { clock.Update(.25f); auto frame=clock.Draw(); drawFog(forest, frame); clock.RenderCompleted(.25f); }
    // Modal suppresses world updates, but the process shader clock keeps rendering.
    for (int i=0; i<2; ++i) { clock.Update(.25f); auto frame=clock.Draw(); drawFog(forest, frame); clock.RenderCompleted(.25f); }
    game::LevelScene nextScene;
    assert(nextScene.Load("Game/XmlFiles/levels/forest_exploration_1.xml", renderer, error));
    const auto beforeUpdate = clock.Draw(); drawFog(nextScene, beforeUpdate);
    near(beforeUpdate.totalTime, 2.25f); assert(beforeUpdate.pixelWobbleTime > 0.f);
    clock.RenderCompleted(0.f);
    clock.Update(.25f); auto afterUpdate = clock.Draw(); drawFog(nextScene, afterUpdate);
    near(afterUpdate.totalTime, 2.5f);
    // Authored foliage in a fresh scene receives the same process time.
    game::LevelScene waterfall;
    assert(waterfall.Load("Game/XmlFiles/levels/waterfall_top.xml", renderer, error));
    renderer.drawn.clear(); waterfall.Draw(renderer, 170.f, 2575.f, 170.f, 2575.f, {}, 1.f, 1.f, afterUpdate);
    bool sawFoliage = false;
    for (const auto& sprite : renderer.drawn) if (sprite.foliageSway.enabled) {
        near(sprite.foliageSway.time, 2.5f); sawFoliage = true;
    }
    assert(sawFoliage);
    // Strict oscillator thresholds and direction (10 still forward, 5 still reverse).
    game::ShaderClock thresholds;
    thresholds.RenderCompleted(1.f);
    for (int i=0; i<10; ++i) { thresholds.Draw(); assert(!thresholds.Reversing()); }
    near(thresholds.Draw().pixelWobbleTime, 11.f); assert(thresholds.Reversing());
    for (int i=0; i<6; ++i) thresholds.Draw();
    near(thresholds.Draw().pixelWobbleTime, 4.f); assert(!thresholds.Reversing());
    near(thresholds.Draw().pixelWobbleTime, 5.f);
    std::cout << "PASS: process shader clock menu/modal/fresh-scene draw seam and strict oscillator boundaries\n";
}
