#include "../game/Hud.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <map>
#include <string>
#include <vector>
using namespace deathleak;
struct Spy final : render::Renderer {
    std::map<std::string, render::TextureId> ids;
    std::vector<render::Sprite> sprites;
    bool Initialize(const char*, int, int, std::string&) override { return true; }
    render::TextureId LoadTexture(const std::string& p, std::string&) override {
        auto id=static_cast<render::TextureId>(ids.size()+1); ids[p]=id; return id;
    }
    void BeginFrame(render::Color) override {}
    void DrawSprite(const render::Sprite& s) override { sprites.push_back(s); }
    void FillRect(render::Rect,render::Color) override {}
    void ApplyScenePostprocess() override {}
    void EndFrame() override {}
    bool PollQuit() override { return false; }
    bool KeyDown(int) const override { return false; }
    render::MouseState Mouse() const override { return {}; }
    bool SaveFrame(const std::string&,std::string&) override { return false; }
};
static void near(float a,float b) { assert(std::fabs(a-b)<.001f); }
static void check(const Spy& spy, const std::string& texture, float y, int count) {
    int seen=0;
    for (const auto& s: spy.sprites) if (s.texture==spy.ids.at(texture)) {
        near(s.destination.y,y); near(s.destination.height,19.f*2.f/3.f); ++seen;
    }
    assert(seen==count);
}
static void frame(const Spy& spy, bool backing) {
    constexpr float healthY=(540.f-460.f-19.f)*2.f/3.f;
    constexpr float focusY=(540.f-440.f-19.f)*2.f/3.f;
    check(spy,"Game/Media/UI/meter_bar_middle.png",healthY,1);
    check(spy,"Game/Media/UI/meter_bar_middle_back.png",healthY,backing?1:0);
    check(spy,"Game/Media/UI/xp_meter_bar_middle.png",focusY,1);
    const auto edge=spy.ids.at("Game/Media/UI/meter_edge_2.png");
    const auto overlay=spy.ids.at("Game/Media/UI/meter_overlay.png");
    int healthCaps=0,focusCaps=0,healthOverlay=0,focusOverlay=0;
    for (const auto& s: spy.sprites) {
        if (s.texture==edge) {
            if (healthCaps<2) { near(s.destination.y,healthY); ++healthCaps; }
            else { near(s.destination.y,focusY); ++focusCaps; }
        }
        if (s.texture==overlay) {
            if (!healthOverlay) { near(s.destination.y,healthY); ++healthOverlay; }
            else { near(s.destination.y,focusY); ++focusOverlay; }
            near(s.destination.height,19.f*2.f/3.f);
        }
    }
    assert(healthCaps==2 && focusCaps==2 && healthOverlay==1 && focusOverlay==1);
}
int main() {
    Spy spy; game::Hud hud; hud.Load(spy);
    hud.Draw(spy,25,25,100,5,0); frame(spy,false);
    spy.sprites.clear(); hud.Draw(spy,25,25,100,5,1.f/60); frame(spy,false);
    spy.sprites.clear(); hud.Draw(spy,12.5f,25,50,5,1.f); frame(spy,false);
    spy.sprites.clear(); hud.Draw(spy,12.5f,25,50,5,1.1f); frame(spy,true);
    std::cout << "HUD vertical stack source projection PASS\n";
}
