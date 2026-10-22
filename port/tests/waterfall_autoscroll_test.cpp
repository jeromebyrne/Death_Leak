#include "../game/LevelScene.h"
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <map>

class CaptureRenderer final : public deathleak::render::Renderer {
public:
    std::vector<deathleak::render::Sprite> drawn;
    bool Initialize(const char*, int, int, std::string&) override { return true; }
    deathleak::render::TextureId LoadTexture(const std::string&, std::string&) override { return 1; }
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
    setenv("DEATHLEAK_RENDER_ONLY_XML_IDS", "6,7,10,13,14,15,16,31", 1);
    CaptureRenderer renderer;
    deathleak::game::LevelScene scene;
    std::string error;
    assert(scene.Load("Game/XmlFiles/levels/waterfall_top.xml", renderer, error));
    auto draw = [&](float cameraY) {
        renderer.drawn.clear();
        scene.Draw(renderer, 300.f, cameraY, 300.f, 2600.f);
        assert(renderer.drawn.size() == 8);
        return renderer.drawn;
    };
    // Depth-descending painter sequence for these authored sheets. Distinct XY
    // and alpha values also catch omitted/duplicated sheets at the fixed camera.
    const int speeds[] = {7,11,11,11,11,9,9,7};
    const auto initial = draw(2600.f);
    for (const auto& s : initial) assert(std::abs(s.uvShiftY) < 1e-6f);
    const auto redraw = draw(2600.f);
    for (size_t i=0;i<8;++i) assert(std::abs(redraw[i].uvShiftY-initial[i].uvShiftY)<1e-6f);
    scene.UpdateVisuals(1.f/60.f, 300.f, 2600.f, 300.f, 2600.f);
    const auto first = draw(2600.f);
    for (size_t i=0;i<8;++i) {
        assert(std::abs(first[i].uvShiftY-speeds[i]*(2.f/3.f)*.001f)<1e-5f);
        assert(first[i].destination.x == initial[i].destination.x);
        assert(first[i].destination.y == initial[i].destination.y);
        assert(first[i].destination.width == initial[i].destination.width);
        assert(first[i].destination.height == initial[i].destination.height);
        assert(first[i].opacity == initial[i].opacity);
        assert(first[i].tint.a == initial[i].tint.a);
        assert(first[i].flipY == initial[i].flipY);
    }
    for (int tick=1;tick<60;++tick) scene.UpdateVisuals(1.f/60.f, 300.f, 2600.f, 300.f, 2600.f);
    const auto sixty = draw(2600.f);
    for (size_t i=0;i<8;++i)
        assert(std::abs(sixty[i].uvShiftY-speeds[i]*(2.f/3.f)*.06f)<1e-4f);
    const auto cameraMoved = draw(2700.f);
    for (size_t i=0;i<8;++i) assert(std::abs(cameraMoved[i].uvShiftY-sixty[i].uvShiftY)<1e-6f);
    // Phase belongs to update, not draw/camera, and wraps without accumulating
    // unbounded values over long sessions.
    for (int tick=60;tick<600;++tick) scene.UpdateVisuals(1.f/60.f, 300.f, 2600.f, 300.f, 2600.f);
    const auto later = draw(2600.f);
    for (size_t i=0;i<8;++i) {
        float expected = std::fmod(speeds[i]*(2.f/3.f)*.001f*600.f, 1.f);
        assert(std::abs(later[i].uvShiftY-expected)<1e-3f);
    }
    std::cout << "PASS: eight authored waterfall layers retain painter/destination/alpha, scaled 7:9:11 V rate, redraw and camera independence, wrap\n";
}
