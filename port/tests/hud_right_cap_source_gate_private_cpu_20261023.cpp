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
    const render::Sprite& rightCap(float y) const {
        const auto id=ids.at("Game/Media/UI/meter_edge_2.png");
        const render::Sprite* found=nullptr;
        for (const auto& s: sprites) if (s.texture==id && std::fabs(s.destination.y-y)<.01f && s.destination.x>60) {
            assert(found==nullptr); found=&s;
        }
        assert(found); return *found;
    }
};
static void near(float actual,float expected) { assert(std::fabs(actual-expected)<.001f); }
static void check(const Spy& spy,float healthLength) {
    // XML x=-900, width=300/200 in centered 1920 UI. UIMeter uses
    // initial x + dynamic meter length for each right edge, not x-edgeWidth.
    const auto& h=spy.rightCap(42);
    near(h.destination.x,(960.f-900.f+healthLength)*2.f/3.f);
    near(h.destination.y,42); near(h.destination.width,4); near(h.destination.height,15);
    assert(h.tint.a==220 && h.opacity==1 && h.rotationDegrees==0 && h.flipX && !h.flipY);
    const auto& f=spy.rightCap(63);
    near(f.destination.x,(960.f-900.f+200.f)*2.f/3.f);
    near(f.destination.y,63); near(f.destination.width,4); near(f.destination.height,15);
    // Authored focus vflip=true persists across UIMeter Draw; right edge adds hflip=true.
    assert(f.tint.a==220 && f.opacity==1 && f.rotationDegrees==0 && f.flipX && f.flipY);
    assert(!f.repeatX && f.source.width==0 && f.source.height==0);
}
int main() {
    Spy spy; game::Hud hud; hud.Load(spy);
    hud.Draw(spy,25,25,100,5,0); check(spy,300);
    spy.sprites.clear(); hud.Draw(spy,25,25,100,5,1.f/60); check(spy,75);
    spy.sprites.clear(); hud.Draw(spy,12.5f,25,50,5,1.f); check(spy,75);
    std::cout << "HUD right-cap source geometry and focus aggregate PASS\n";
}
