#include "../game/LevelScene.h"
#include "../linux/FoliageSway.h"
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>

using deathleak::render::Sprite;
using deathleak::render::FoliageSwayWorldVertex;
class CaptureRenderer final : public deathleak::render::Renderer {
public:
    std::vector<Sprite> drawn;
    bool Initialize(const char*, int, int, std::string&) override { return true; }
    deathleak::render::TextureId LoadTexture(const std::string&, std::string&) override { return 1; }
    void BeginFrame(deathleak::render::Color) override {}
    void DrawSprite(const Sprite& s) override { drawn.push_back(s); }
    void FillRect(deathleak::render::Rect, deathleak::render::Color) override {}
    void ApplyScenePostprocess() override {}
    void EndFrame() override {}
    bool PollQuit() override { return false; }
    bool KeyDown(int) const override { return false; }
    deathleak::render::MouseState Mouse() const override { return {}; }
    bool SaveFrame(const std::string&, std::string&) override { return false; }
};
static bool near(float a, float b, float tolerance = 1e-4f) {
    return std::fabs(a-b) < tolerance;
}
int main() {
    setenv("DEATHLEAK_RENDER_ONLY_XML_IDS", "36,60", 1);
    CaptureRenderer renderer;
    deathleak::game::LevelScene scene;
    std::string error;
    assert(scene.Load("Game/XmlFiles/levels/waterfall_top.xml", renderer, error));
    scene.UpdateVisuals(.75f, 170.f, 2575.f, 170.f, 2575.f);
    for (const auto& fixture : {std::pair{"36", 6.f}, std::pair{"60", 1.f}}) {
        const bool rope = fixture.first[0] == '3';
        const float cameraX = rope ? 170.f : -2063.f;
        const float cameraY = rope ? 2575.f : 3571.f;
        auto draw = [&](float global) {
            renderer.drawn.clear();
            scene.Draw(renderer, cameraX, cameraY, cameraX, cameraY, {}, 1.f, global, {.75f, 0.f});
            assert(renderer.drawn.size() == 1);
            return renderer.drawn.front();
        };
        const auto fresh = draw(1.f), rain = draw(2.625f), after = draw(2.5f);
        assert(fresh.foliageSway.enabled);
        assert(near(fresh.foliageSway.timeMultiplier, fixture.second));
        assert(near(fresh.foliageSway.time, .75f));
        assert(near(fresh.foliageSway.globalTimeMultiplier, 1.f));
        assert(near(rain.foliageSway.globalTimeMultiplier, 2.625f));
        assert(near(after.foliageSway.globalTimeMultiplier, 2.5f));
        assert(near(fresh.foliageSway.bottomLeft.x, rope ? 10.f : 20.f));
        assert(near(fresh.foliageSway.bottomLeft.y, 0.f));
        assert(near(rain.foliageSway.bottomLeft.x, fresh.foliageSway.bottomLeft.x));
        assert(fresh.destination.x == rain.destination.x && fresh.destination.y == rain.destination.y);
        assert(fresh.texture == rain.texture && fresh.tint.a == rain.tint.a);
        assert(fresh.flipX == rain.flipX && fresh.flipY == rain.flipY);
        const auto& sway = fresh.foliageSway;
        const deathleak::render::SwayPoint vertex{sway.worldX, sway.worldY};
        const deathleak::render::SwayPoint normal{sway.bottomLeft.x, sway.bottomLeft.y};
        const auto p1 = FoliageSwayWorldVertex(vertex, normal, sway.time, sway.timeMultiplier, fresh.foliageSway.globalTimeMultiplier);
        const auto p2 = FoliageSwayWorldVertex(vertex, normal, sway.time, sway.timeMultiplier, rain.foliageSway.globalTimeMultiplier);
        const auto p3 = FoliageSwayWorldVertex(vertex, normal, sway.time, sway.timeMultiplier, after.foliageSway.globalTimeMultiplier);
        assert(std::fabs(p1.x-p2.x) > .01f && std::fabs(p2.x-p3.x) > .01f);
        assert(near(p1.x, vertex.x + std::sin(vertex.y*.1f + sway.time*sway.timeMultiplier)*normal.x));
        assert(FoliageSwayWorldVertex(vertex, {0,0}, sway.time, sway.timeMultiplier, 2.625f).x == vertex.x);
    }
    const auto p = FoliageSwayWorldVertex({100.f, 30.f}, {7.f, 6.f}, .75f, 6.f, 2.625f);
    assert(near(p.y, 30.f + std::sin(p.x*.1f + .75f*6.f*2.625f)*6.f));
    std::cout << "PASS: authored rope/tree foliage weather phase propagates to sprite and ordered shader displacement\n";
}
