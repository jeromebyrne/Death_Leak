#include "../game/Hud.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>
using namespace deathleak;
namespace {
constexpr float kBar = (75.f - 3.2f) * 2.f / 3.f;
bool near(float a, float b) { return std::fabs(a-b) < .0006f; }
struct Spy final : render::Renderer {
    std::vector<render::Sprite> sprites;
    std::vector<render::Rect> glyphs;
    bool Initialize(const char*, int, int, std::string&) override { return true; }
    render::TextureId LoadTexture(const std::string& path, std::string&) override {
        if (path.find("meter_bar_middle_back") != std::string::npos) return 6;
        if (path.find("xp_meter_bar_middle") != std::string::npos) return 3;
        if (path.find("meter_bar_middle") != std::string::npos) return 2;
        if (path.find("meter_edge_2") != std::string::npos) return 1;
        if (path.find("meter_overlay") != std::string::npos) return 4;
        if (path.find("orb_count") != std::string::npos) return 5;
        assert(false); return 0;
    }
    void BeginFrame(render::Color) override {}
    void DrawSprite(const render::Sprite& s) override { sprites.push_back(s); }
    void FillRect(render::Rect r, render::Color) override { glyphs.push_back(r); }
    void ApplyScenePostprocess() override {}
    void EndFrame() override {}
    bool PollQuit() override { return false; }
    bool KeyDown(int) const override { return false; }
    render::MouseState Mouse() const override { return {}; }
    bool SaveFrame(const std::string&, std::string&) override { return false; }
    void clear() { sprites.clear(); glyphs.clear(); }
    int index(unsigned id) const { for (size_t i=0;i<sprites.size();++i) if(sprites[i].texture==id) return static_cast<int>(i); return -1; }
    const render::Sprite& at(unsigned id) const { int i=index(id); assert(i>=0); return sprites[i]; }
};
void draw(Spy& spy, game::Hud& hud, float t, float health, float focus=100.f) {
    spy.clear(); hud.Draw(spy, health, 25.f, focus, 12, t);
}
void check(const Spy& spy, float health, float focus, float backing=-1.f) {
    assert(near(spy.at(2).destination.width, kBar*health));
    assert(near(spy.at(3).destination.width, 127.f*focus));
    assert((spy.index(6)>=0) == (backing>=0.f));
    if (backing>=0.f) {
        const auto& b=spy.at(6); const auto& red=spy.at(2);
        assert(spy.index(6)+1 == spy.index(2));
        assert(near(b.destination.x,43) && near(b.destination.y,45));
        assert(near(b.destination.height,9) && near(b.destination.width,kBar*backing));
        assert(b.tint.r==red.tint.r && b.tint.g==red.tint.g && b.tint.b==red.tint.b && b.tint.a==red.tint.a);
    }
    assert(spy.index(5)==0 && spy.index(2)>spy.index(5));
    assert(spy.glyphs.size()>0);
    assert(spy.at(4).destination.x==40 && near(spy.at(4).destination.width,50));
    assert(spy.at(3).destination.x==43 && spy.at(3).destination.y==66);
    int focusOverlay=0, healthRight=0, focusRight=0;
    for(const auto& s:spy.sprites) {
        if(s.texture==4 && s.destination.y==64 && near(s.destination.width,133)) ++focusOverlay;
        if(s.texture==1 && s.destination.y==42 && s.flipX && near(s.destination.x,87)) ++healthRight;
        if(s.texture==1 && s.destination.y==63 && near(s.destination.x,170)) ++focusRight;
    }
    assert(focusOverlay==1 && healthRight==1 && focusRight==1);
}
void settled(Spy& spy, game::Hud& hud) { draw(spy,hud,0,25); draw(spy,hud,1.f/60,25); check(spy,1,1); }
}
int main() {
    Spy spy; game::Hud hud; hud.Load(spy); settled(spy,hud);
    const float t=1.f;
    draw(spy,hud,t,20); check(spy,1,1);
    draw(spy,hud,t+1.f/60,20); check(spy,1-.2f*(1.f/60)/.25f,1,1);
    draw(spy,hud,t+.125f,20); check(spy,.9f,1,1);
    draw(spy,hud,t+.25f,20); check(spy,.8f,1,1);
    draw(spy,hud,t+.26f,20); check(spy,.8f,1,.995f);
    // Different time intervals still decay by one fixed .005 step per update.
    draw(spy,hud,t+5.f,20); check(spy,.8f,1,.99f);
    for (int i=3;i<=40;++i) {
        draw(spy,hud,t+5.f+i*.0001f,20);
        check(spy,.8f,1,1.f-.005f*i);
    }
    draw(spy,hud,t+6.f,20); check(spy,.8f,1); // undershoot corrected

    game::Hud focusHud; focusHud.Load(spy); settled(spy,focusHud);
    draw(spy,focusHud,1,25,50); check(spy,1,1);
    draw(spy,focusHud,1+1.f/60,25,50); check(spy,1,.5f);
    draw(spy,focusHud,2,25,100); check(spy,1,.5f);
    draw(spy,focusHud,2.125f,25,100); check(spy,1,.75f);
    draw(spy,focusHud,2.25f,25,100); check(spy,1,1.f);

    game::Hud retarget; retarget.Load(spy); settled(spy,retarget);
    draw(spy,retarget,1,20); check(spy,1,1);
    draw(spy,retarget,1.0625f,15); check(spy,.95f,1,1);
    draw(spy,retarget,1.125f,15); check(spy,.8f,1,1); // original baseline and start
    // Equal-to-displayed target cannot replace the active target.
    draw(spy,retarget,1.125f,20); check(spy,.8f,1,1);
    draw(spy,retarget,1.15f,15); check(spy,.76f,1,1);
    draw(spy,retarget,1.25f,15); check(spy,.6f,1,1);

    game::Hud focusRetarget; focusRetarget.Load(spy); settled(spy,focusRetarget);
    draw(spy,focusRetarget,1,25,50);
    draw(spy,focusRetarget,1.01f,25,50); check(spy,1,.5f);
    draw(spy,focusRetarget,2,25,100); check(spy,1,.5f);
    draw(spy,focusRetarget,2.125f,25,100); check(spy,1,.75f);
    draw(spy,focusRetarget,2.15f,25,40); check(spy,1,.8f); // retarget below original .5 baseline
    draw(spy,focusRetarget,2.16f,25,40); check(spy,1,.4f); // baseline-based snap
    std::cout << "HUD transition CPU spy PASS\n";
}
