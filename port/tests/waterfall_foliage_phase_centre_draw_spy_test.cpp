// CPU-only integration gate for active waterfall-top ordinary foliage sprites.
// One XML ID per process: LevelScene's diagnostic filter is process-static.
#include "../game/LevelScene.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

using namespace deathleak;
namespace {
struct Spy final : render::Renderer {
    std::vector<render::Sprite> calls;
    bool Initialize(const char*, int, int, std::string&) override { return true; }
    render::TextureId LoadTexture(const std::string&, std::string&) override { return 1; }
    void BeginFrame(render::Color) override {}
    void DrawSprite(const render::Sprite& sprite) override { calls.push_back(sprite); }
    void FillRect(render::Rect, render::Color) override {}
    void ApplyScenePostprocess() override {}
    void EndFrame() override {}
    bool PollQuit() override { return false; }
    bool KeyDown(int) const override { return false; }
    render::MouseState Mouse() const override { return {}; }
    bool SaveFrame(const std::string&, std::string&) override { return false; }
};
struct Case { const char* id; float x, y, width, height, layerX, layerY, cx[2], cy[2]; bool flipX; };
constexpr Case cases[] = {
    {"64", -1179.014648f, 3660.f, 2048.f, 677.f, 1.1f, 1.05f, {0.f,200.f}, {3000.f,3200.f}, false},
    {"65", 2059.985352f, 3706.f, 2048.f, 677.f, 1.1f, 1.05f, {1000.f,1200.f}, {3000.f,3200.f}, false},
    {"60", -2063.014648f, 3571.f, 756.f, 677.f, 1.f, 1.f, {-1500.f,-1300.f}, {3000.f,3200.f}, true},
};
bool near(float a, float b, float tolerance = .025f) { return std::fabs(a-b) <= tolerance; }
bool require(bool condition, const std::string& what) {
    if (!condition) std::cerr << "FAIL " << what << '\n';
    return condition;
}
}
int main(int argc, char** argv) {
    if (argc != 3 || (std::string(argv[2]) != "baseline" && std::string(argv[2]) != "fixed")) {
        std::cerr << "usage: waterfall_foliage_phase_centre_draw_spy_test ID baseline|fixed\n";
        return 2;
    }
    const Case* target = nullptr;
    for (const auto& c : cases) if (argv[1] == std::string(c.id)) target = &c;
    if (!target) { std::cerr << "unknown ID\n"; return 2; }
    const bool fixed = std::string(argv[2]) == "fixed";
    setenv("DEATHLEAK_RENDER_ONLY_XML_IDS", target->id, 1);
    Spy spy; game::LevelScene scene; std::string error;
    if (!scene.Load("Game/XmlFiles/levels/waterfall_top.xml", spy, error)) {
        std::cerr << "LOAD FAIL " << error << '\n'; return 2;
    }
    bool ok = true;
    for (int camera = 0; camera != 2; ++camera) {
        for (float time : {0.f, .5f}) {
            spy.calls.clear();
            const float cx = target->cx[camera], cy = target->cy[camera];
            scene.Draw(spy, cx, cy, cx, cy, {}, 1.f, 1.f, {time, 0.f});
            if (!require(spy.calls.size() == 1, std::string("ID")+target->id+" submitted exactly once")) { ok = false; continue; }
            const auto& s = spy.calls[0];
            const float expectedX = target->x - (cx-target->x)*(target->layerX-1.f);
            const float expectedY = target->y - (cy-target->y)*(target->layerY-1.f);
            const float destinationX = 640.f + ((target->x-cx)*target->layerX-target->width/2.f)*2.f/3.f;
            const float destinationY = 360.f + ((cy-target->y)*target->layerY-target->height/2.f)*2.f/3.f;
            const bool fields = s.foliageSway.enabled && s.texture == 1 &&
                near(s.destination.x,destinationX) && near(s.destination.y,destinationY) &&
                near(s.destination.width,target->width*2.f/3.f) && near(s.destination.height,target->height*2.f/3.f) &&
                near(s.foliageSway.bottomLeft.x,20.f) && near(s.foliageSway.bottomRight.x,20.f) &&
                near(s.foliageSway.topLeft.x,0.f) && near(s.foliageSway.topRight.x,0.f) &&
                near(s.foliageSway.bottomLeft.y,0.f) && near(s.foliageSway.time,time) &&
                near(s.foliageSway.timeMultiplier,1.f) && near(s.foliageSway.globalTimeMultiplier,1.f) &&
                s.flipX == target->flipX && near(s.rotationDegrees,0.f);
            ok &= require(fields, std::string("ID")+target->id+" destination/authored fields");
            const bool centre = near(s.foliageSway.worldX,expectedX) && near(s.foliageSway.worldY,expectedY);
            const bool expectedCentre = fixed || std::string(target->id) == "60";
            ok &= require(centre == expectedCentre, std::string("ID")+target->id+" centre expectation");
            // Independent shader equation: source-bottom (not screen-left/right on flipped ID60).
            const float actualDx = 20.f*std::sin(.1f*(s.foliageSway.worldY-target->height/2.f)+time);
            const float desiredDx = 20.f*std::sin(.1f*(expectedY-target->height/2.f)+time);
            const bool displacementMatches = near(actualDx, desiredDx, .06f);
            ok &= require(displacementMatches == expectedCentre, std::string("ID")+target->id+" source-bottom phase");
            std::cout << "ID" << target->id << " camera=" << camera << " time=" << time
                      << " centre=" << (centre?"MATCH":"MISMATCH")
                      << " bottom_phase=" << (displacementMatches?"MATCH":"MISMATCH") << '\n';
        }
    }
    std::cout << "FOLIAGE_PHASE_DRAW_SPY=" << (ok?"PASS":"FAIL") << " mode=" << argv[2] << " ID=" << target->id << '\n';
    return ok ? 0 : 1;
}
