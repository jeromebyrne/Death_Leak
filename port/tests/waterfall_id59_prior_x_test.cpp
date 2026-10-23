#include "../game/LevelScene.h"
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>
using namespace deathleak;
struct Spy final : render::Renderer {
    std::vector<render::Sprite> calls;
    bool Initialize(const char*, int, int, std::string&) override { return true; }
    render::TextureId LoadTexture(const std::string&, std::string&) override { return 1; }
    void BeginFrame(render::Color) override {}
    void DrawSprite(const render::Sprite& s) override { calls.push_back(s); }
    void FillRect(render::Rect, render::Color) override {}
    void ApplyScenePostprocess() override {}
    void EndFrame() override {}
    bool PollQuit() override { return false; }
    bool KeyDown(int) const override { return false; }
    render::MouseState Mouse() const override { return {}; }
    bool SaveFrame(const std::string&, std::string&) override { return false; }
};
static void near(float a, float b) { if (std::fabs(a-b) > .005f) { std::cerr << a << " != " << b << '\n'; std::abort(); } }
int main() {
    setenv("DEATHLEAK_RENDER_ONLY_XML_IDS", "59", 1);
    Spy spy; game::LevelScene scene; std::string error;
    assert(scene.Load("Game/XmlFiles/levels/waterfall_top.xml", spy, error));
    auto update = [&](float camera, bool paused=false) { scene.UpdateVisuals(1.f/60.f, scene.PlayerX(), scene.PlayerY(), camera, 3000.f, paused); };
    auto draw = [&](float camera, float world) {
        spy.calls.clear(); scene.Draw(spy, camera, 3000.f, scene.PlayerX(), scene.PlayerY());
        assert(spy.calls.size()==1); const auto& s=spy.calls.front();
        near(s.destination.x, 640.f + (world-camera-1024.f)*2.f/3.f);
        near(s.uvShiftX, camera*.000690f);
        near(s.destination.width, 2048.f*2.f/3.f);
        near(s.destination.height, 512.f*2.f/3.f);
        near(s.opacity, .95f);
    };
    update(1600.f); draw(1600.f,2778.985352f);
    update(1612.f); draw(1612.f,1600.f);
    update(1620.f); draw(1620.f,1612.f);
    update(1640.f,true); draw(1640.f,1612.f); draw(1640.f,1612.f);
    update(1650.f); draw(1650.f,1620.f);
    assert(scene.Load("Game/XmlFiles/levels/waterfall_top.xml", spy, error));
    update(1600.f); update(1612.f); draw(1620.f,1600.f);
    assert(scene.Load("Game/XmlFiles/levels/waterfall_top.xml", spy, error));
    update(1600.f); draw(1600.f,2778.985352f);
    std::cout << "PASS ID59 prior-update world X, current UV, paused/repeated draw, multi-update, reload\n";
}
