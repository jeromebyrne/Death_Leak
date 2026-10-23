#include "../game/Hud.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <map>
#include <string>
#include <vector>
using namespace deathleak;
struct Spy final : render::Renderer {
    std::map<std::string,render::TextureId> ids;
    std::vector<render::Sprite> sprites;
    bool Initialize(const char*,int,int,std::string&) override { return true; }
    render::TextureId LoadTexture(const std::string& p,std::string&) override { auto id=static_cast<render::TextureId>(ids.size()+1); ids[p]=id; return id; }
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
static void near(float a,float b) { if (std::fabs(a-b)>=.001f) { std::cerr<<"actual="<<a<<" expected="<<b<<"\n"; std::abort(); } }
static std::vector<render::Sprite> find(const Spy& s,const char* name) {
    std::vector<render::Sprite> v;
    for (const auto& d:s.sprites) if (d.texture==s.ids.at(name)) v.push_back(d);
    return v;
}
static void check(const Spy& s,float length,float healthFill,float focusFill,bool backing) {
    constexpr float scale=2.f/3.f;
    const float barX=40.f+3.6f*scale, middleX=40.f+4.f*scale;
    auto health=find(s,"Game/Media/UI/meter_bar_middle.png"); assert(health.size()==1);
    near(health[0].destination.x,barX); near(health[0].destination.width,(length-3.2f)*scale*healthFill);
    auto focus=find(s,"Game/Media/UI/xp_meter_bar_middle.png"); assert(focus.size()==1);
    near(focus[0].destination.x,barX); near(focus[0].destination.width,(200.f-3.2f)*scale*focusFill);
    auto back=find(s,"Game/Media/UI/meter_bar_middle_back.png"); assert(back.size()==(backing?1u:0u));
    if (backing) { near(back[0].destination.x,barX); near(back[0].destination.width,(length-3.2f)*scale); }
    auto middle=find(s,"Game/Media/UI/meter_overlay.png"); assert(middle.size()==2);
    near(middle[0].destination.x,middleX); near(middle[0].destination.width,(length-3.8f)*scale);
    near(middle[1].destination.x,middleX); near(middle[1].destination.width,(200.f-3.8f)*scale);
}
int main() {
    Spy spy; game::Hud hud; hud.Load(spy);
    hud.Draw(spy,25,25,100,5,0.f); check(spy,300.f,1.f,1.f,false);
    spy.sprites.clear(); hud.Draw(spy,25,25,100,5,1.f/60.f); check(spy,75.f,1.f,1.f,false);
    spy.sprites.clear(); hud.Draw(spy,12.5f,25,50,5,1.f); check(spy,75.f,1.f,1.f,false);
    spy.sprites.clear(); hud.Draw(spy,12.5f,25,50,5,1.1f); check(spy,75.f,.8f,.5f,true);
    std::cout<<"HUD horizontal meter source projection PASS\n";
}
