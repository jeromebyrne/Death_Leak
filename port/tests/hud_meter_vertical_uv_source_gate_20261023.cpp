#include "../game/Hud.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <map>
#include <string>
#include <vector>
using namespace deathleak;
namespace {
bool near(float a, float b) { return std::fabs(a-b) < .0006f; }
struct Spy final : render::Renderer {
    std::map<std::string,render::TextureId> ids;
    std::vector<render::Sprite> sprites;
    std::vector<render::Rect> glyphs;
    bool Initialize(const char*,int,int,std::string&) override { return true; }
    render::TextureId LoadTexture(const std::string& path,std::string&) override {
        auto id=static_cast<render::TextureId>(ids.size()+1);
        assert(ids.emplace(path,id).second); return id;
    }
    void BeginFrame(render::Color) override {}
    void DrawSprite(const render::Sprite& sprite) override { sprites.push_back(sprite); }
    void FillRect(render::Rect rect,render::Color) override { glyphs.push_back(rect); }
    void ApplyScenePostprocess() override {}
    void EndFrame() override {}
    bool PollQuit() override { return false; }
    bool KeyDown(int) const override { return false; }
    render::MouseState Mouse() const override { return {}; }
    bool SaveFrame(const std::string&,std::string&) override { return false; }
    render::TextureId id(const char* name) const { return ids.at(std::string("Game/Media/UI/")+name); }
    void clear() { sprites.clear(); glyphs.clear(); }
};
void check(const Spy& spy,float focusFraction) {
    assert(spy.ids.size()==6);
    assert(spy.sprites.size()==9);
    assert(!spy.glyphs.empty());
    int focusCount=0;
    for (const auto& s:spy.sprites) {
        if (s.texture==spy.id("xp_meter_bar_middle.png")) {
            ++focusCount;
            assert(s.flipY && !s.flipX && !s.repeatX);
            assert(s.source.x==0 && s.source.y==0 && s.source.width==0 && s.source.height==0);
            assert(near(s.destination.x,42.4f) && near(s.destination.y,54.f));
            assert(near(s.destination.width,(200.f-4.f*.8f)*(2.f/3.f)*focusFraction) && s.destination.width>0);
            assert(near(s.destination.height,19.f*(2.f/3.f)));
            assert(s.tint.r==255 && s.tint.g==255 && s.tint.b==255 && s.tint.a==220);
            assert(near(s.opacity,1) && near(s.rotationDegrees,0));
            assert(!s.foliageSway.enabled && !s.pixelWobble.enabled);
            assert(near(s.uvShiftX,0) && near(s.uvShiftY,0) && near(s.rgbGain,1));
        }
    }
    assert(focusCount==1);
    assert(spy.sprites[5].texture==spy.id("xp_meter_bar_middle.png"));
    // Original UIMeter preserves the XML vflip across each focus component.
    // Health XML is not vertically flipped; only the focus stack is.
    for (int i=0;i<5;++i) assert(!spy.sprites[i].flipY);
    for (int i=5;i<9;++i) assert(spy.sprites[i].flipY);
    assert(spy.sprites[6].texture==spy.id("meter_edge_2.png") && !spy.sprites[6].flipX);
    assert(spy.sprites[7].texture==spy.id("meter_edge_2.png") && spy.sprites[7].flipX);
    assert(spy.sprites[8].texture==spy.id("meter_overlay.png") && !spy.sprites[8].flipX);
}
}
int main() {
    Spy spy; game::Hud hud; hud.Load(spy);
    hud.Draw(spy,25,25,100,12,0); check(spy,1); // source's initial positive fill
    spy.clear(); hud.Draw(spy,25,25,100,12,1.f/60); check(spy,1);
    spy.clear(); hud.Draw(spy,25,25,50,12,1); check(spy,1); // registration precedes transition
    spy.clear(); hud.Draw(spy,25,25,50,12,1+1.f/60); check(spy,.5f); // snap decrease
    spy.clear(); hud.Draw(spy,25,25,100,12,2); check(spy,.5f);
    spy.clear(); hud.Draw(spy,25,25,100,12,2.125f); check(spy,.75f);
    std::cout << "HUD meter vertical UV source gate PASS\n";
}
