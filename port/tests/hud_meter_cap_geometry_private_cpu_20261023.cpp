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
static void check(const Spy& spy,float healthLength) {
    std::vector<render::Sprite> caps;
    for (const auto& s: spy.sprites)
        if (s.texture==spy.ids.at("Game/Media/UI/meter_edge_2.png")) caps.push_back(s);
    assert(caps.size()==4);
    const float expectedX[] = {40.f, (60.f+healthLength)*2.f/3.f,
                               40.f, (60.f+200.f)*2.f/3.f};
    const float expectedY[] = {42.f,42.f,63.f,63.f};
    for (int i=0;i<4;++i) {
        near(caps[i].destination.x,expectedX[i]);
        near(caps[i].destination.y,expectedY[i]);
        near(caps[i].destination.width,4.f*2.f/3.f);
        near(caps[i].destination.height,19.f*2.f/3.f);
        assert(caps[i].tint.a==220 && caps[i].opacity==1.f);
        assert(caps[i].rotationDegrees==0.f);
        assert(caps[i].flipX==(i%2==1));
        assert(caps[i].flipY==(i>=3));
    }
}
int main() {
    Spy spy; game::Hud hud; hud.Load(spy);
    hud.Draw(spy,25,25,100,5,0); check(spy,300);
    spy.sprites.clear(); hud.Draw(spy,25,25,100,5,1.f/60); check(spy,75);
    spy.sprites.clear(); hud.Draw(spy,12.5f,25,50,5,1.f); check(spy,75);
    std::cout << "HUD meter cap X/size/flip geometry PASS\n";
}
