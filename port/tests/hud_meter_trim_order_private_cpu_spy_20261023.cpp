#include "../game/Hud.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <map>
#include <string>
#include <vector>
using namespace deathleak;
namespace {
struct Spy final : render::Renderer {
    std::map<std::string, render::TextureId> ids;
    std::vector<render::Sprite> sprites;
    bool Initialize(const char*, int, int, std::string&) override { return true; }
    render::TextureId LoadTexture(const std::string& p, std::string&) override {
        const auto id = static_cast<render::TextureId>(ids.size()+1);
        assert(ids.emplace(p,id).second); return id;
    }
    void BeginFrame(render::Color) override {}
    void DrawSprite(const render::Sprite& s) override { sprites.push_back(s); }
    void FillRect(render::Rect, render::Color) override {}
    void ApplyScenePostprocess() override {}
    void EndFrame() override {}
    bool PollQuit() override { return false; }
    bool KeyDown(int) const override { return false; }
    render::MouseState Mouse() const override { return {}; }
    bool SaveFrame(const std::string&, std::string&) override { return false; }
    render::TextureId id(const char* p) const { return ids.at(std::string("Game/Media/UI/")+p); }
};
bool near(float a,float b) { return std::fabs(a-b)<0.0005f; }
void sprite(const Spy& spy, size_t i, const char* texture, render::Rect d,
            unsigned alpha, bool flipX=false, bool flipY=false, float opacity=1.f, float rotation=0.f) {
    const auto& s=spy.sprites.at(i);
    assert(s.texture==spy.id(texture));
    assert(near(s.destination.x,d.x) && near(s.destination.y,d.y));
    if (!near(s.destination.width,d.width) || !near(s.destination.height,d.height)) std::cerr << "size mismatch i=" << i << " tex=" << texture << " actual=" << s.destination.width << "," << s.destination.height << " expected=" << d.width << "," << d.height << "\n";
    assert(near(s.destination.width,d.width) && near(s.destination.height,d.height));
    assert(s.source.x==0 && s.source.y==0 && s.source.width==0 && s.source.height==0);
    assert(s.tint.r==255 && s.tint.g==255 && s.tint.b==255 && s.tint.a==alpha);
    assert(s.flipX==flipX && s.flipY==flipY && !s.repeatX);
    assert(near(s.opacity,opacity) && near(s.rotationDegrees,rotation));
    assert(near(s.uvShiftX,0) && near(s.uvShiftY,0) && near(s.rgbGain,1));
    assert(!s.foliageSway.enabled && !s.pixelWobble.enabled);
}
void check(const Spy& spy,float healthFill,float focusFill,bool loss,float healthWidth) {
    const float healthBarWidth=healthWidth-3.2f*(2.f/3.f);
    assert(spy.ids.size()==6);
    assert(spy.sprites.size()==(loss?10u:9u));
    size_t i=0;
    sprite(spy,i++,"orb_count.png",{40,77.333333f,67.333333f,42.666667f},191);
    if (loss) sprite(spy,i++,"meter_bar_middle_back.png",{43,45,healthBarWidth,9},220);
    sprite(spy,i++,"meter_bar_middle.png",{43,45,healthBarWidth*healthFill,9},220);
    sprite(spy,i++,"meter_edge_2.png",{39,42,4,15},220);
    sprite(spy,i++,"meter_edge_2.png",{37+healthWidth,42,4,15},220,true);
    sprite(spy,i++,"meter_overlay.png",{40,43,healthWidth,13},178);
    sprite(spy,i++,"xp_meter_bar_middle.png",{43,66,127*focusFill,9},220,false,true);
    sprite(spy,i++,"meter_edge_2.png",{39,63,4,15},220);
    sprite(spy,i++,"meter_edge_2.png",{170,63,4,15},220,false,false,0.f,1.f);
    sprite(spy,i++,"meter_overlay.png",{40,64,133,13},178);
}
}
int main() {
    Spy spy; game::Hud hud; hud.Load(spy);
    hud.Draw(spy,25,25,100,5,0); check(spy,1,1,false,200);
    spy.sprites.clear(); hud.Draw(spy,25,25,100,5,1.f/60); check(spy,1,1,false,50);
    // Set targets on this frame; source updates progress on the following frame.
    spy.sprites.clear(); hud.Draw(spy,12.5f,25,50,5,1); check(spy,1,1,false,50);
    spy.sprites.clear(); hud.Draw(spy,12.5f,25,50,5,1+1.f/60);
    check(spy,1.f-(1.f/60.f)/.25f*.5f,.5f,true,50);
    spy.sprites.clear(); hud.Draw(spy,12.5f,25,50,5,1.25f); check(spy,.5f,.5f,true,50);
    std::cout << "hud meter painter order and geometry/UV/alpha PASS\n";
}
