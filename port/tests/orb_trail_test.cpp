#include "../game/LevelScene.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
using namespace deathleak;
class CaptureRenderer final : public render::Renderer {
public:
    std::unordered_map<std::string, render::TextureId> ids;
    std::vector<render::Sprite> drawn;
    bool Initialize(const char*, int, int, std::string&) override { return true; }
    render::TextureId LoadTexture(const std::string& path, std::string&) override {
        return ids.emplace(path, static_cast<render::TextureId>(ids.size()+1)).first->second;
    }
    void BeginFrame(render::Color) override {}
    void DrawSprite(const render::Sprite& sprite) override { drawn.push_back(sprite); }
    void FillRect(render::Rect, render::Color) override {}
    void ApplyScenePostprocess() override {}
    void EndFrame() override {}
    bool PollQuit() override { return false; }
    bool KeyDown(int) const override { return false; }
    render::MouseState Mouse() const override { return {}; }
    bool SaveFrame(const std::string&, std::string&) override { return false; }
};
int main(int argc, char** argv) {
    assert(argc==2);
    const bool forest = std::string(argv[1]) == "forest";
    const bool forest2 = std::string(argv[1]) == "forest2";
    const std::string id = forest2 ? "46" : forest ? "70" : "75";
    setenv("DEATHLEAK_RENDER_ONLY_XML_IDS", id.c_str(), 1);
    const std::string level = std::string("Game/XmlFiles/levels/") +
        (forest2 ? "forest_exploration_2.xml" : forest ? "forest_exploration_1.xml" : "waterfall_top.xml");
    const float ox = forest2 ? -1865.014648f : forest ? 335.985352f : -1335.014648f;
    const float oy = forest2 ? 3096.f : forest ? 3213.f : 2995.f;
    CaptureRenderer r;
    game::LevelScene s;
    std::string error;
    assert(s.Load(level,r,error));
    assert(r.ids.count("Game/Media/orb_shatter.png"));
    auto xy = [&]() {
        r.drawn.clear(); s.Draw(r,ox,oy,9000,9000);
        assert(r.drawn.size()==1);
        const auto& d=r.drawn.front();
        return std::pair<float,float>{ox+(d.destination.x+d.destination.width*.5f-640.f)*1.5f,
            oy-(d.destination.y+d.destination.height*.5f-360.f)*1.5f};
    };
    assert(s.OrbTrails().empty());
    s.UpdateAuthoredOrbs(.25f,9000,9000,ox,oy);
    auto hover=xy();
    assert(std::abs(hover.first-ox)<.01f && std::abs(hover.second-oy)<=20.01f);
    assert(s.OrbTrails().empty());
    s.UpdateAuthoredOrbs(0,ox+130,hover.second,ox,oy);
    for (const auto& t : s.OrbTrails()) assert(t.owner != level+"#"+id);
    s.UpdateAuthoredOrbs(0,ox+129.9f,hover.second,ox,oy);
    auto trails=s.OrbTrails();
    auto own=std::find_if(trails.begin(),trails.end(),[&](const auto& t){return t.owner==level+"#"+id;});
    assert(own!=trails.end() && own->attached);
    const auto& spray=*own->spray;
    assert(spray.texture==r.ids.at("Game/Media/orb_shatter.png"));
    assert(spray.depth==3250 && spray.particles.size()==15 && spray.warmed && spray.looping);
    assert(spray.width==3200 && spray.height==1200 && spray.loopTime==-1);
    assert(spray.fadeIn==.15f && spray.fadeOut==.8f && spray.scaleTo==.1f);
    for (int i=0;i<15;++i) {
        const auto& p=spray.particles[i];
        assert(std::abs(p.lifetime-(.35f-.25f/15.f*i))<.00001f);
        assert(p.size>=40 && p.size<=80 && p.speed>=1.5f && p.speed<=3.4f);
        assert(p.brightness>=.8f && p.brightness<=1 && p.gravity==.5f);
        assert(std::abs(p.x-hover.first)<=30 && std::abs(p.y-hover.second)<=30);
    }
    auto armed=xy();
    assert(std::abs(armed.first-hover.first)<.01f);
    s.UpdateAuthoredOrbs(1.f/60.f,ox+110,hover.second,ox,oy);
    s.UpdateAuthoredOrbs(1.f/60.f,ox+110,hover.second,ox,oy);
    auto afterTrack=s.OrbTrails();
    assert(std::count_if(afterTrack.begin(),afterTrack.end(),[&](const auto& t){return t.owner==level+"#"+id;})==1 && std::abs(xy().first-ox)>.1f);
    auto current=xy();
    for (int i=0;i<22;++i) s.UpdateVisuals(1.f/60.f,9000,9000,ox,oy);
    auto recycled=s.OrbTrails();
    auto recycledOwn=std::find_if(recycled.begin(),recycled.end(),[&](const auto& t){return t.owner==level+"#"+id;});
    assert(recycledOwn!=recycled.end() && recycledOwn->spray->particles.size()==15);
    assert(std::abs(recycledOwn->spray->particles[0].startX-current.first)<.01f);
    assert(std::abs(recycledOwn->spray->particles[0].startY-current.second)<.01f);
    std::unordered_set<std::string> collected;
    s.CollectCurrencyOrbs(current.first+100,current.second,collected);
    assert(!collected.count(level+"#"+id));
    s.CollectCurrencyOrbs(current.first+99.9f,current.second,collected);
    assert(collected.count(level+"#"+id));
    auto afterPickup=s.OrbTrails();
    auto picked=std::find_if(afterPickup.begin(),afterPickup.end(),[&](const auto& t){return t.owner==level+"#"+id;});
    assert(picked!=afterPickup.end() && !picked->attached && !picked->spray->looping);
    for(int i=0;i<30;++i) s.UpdateVisuals(1.f/60.f,9000,9000,ox,oy);
    for (const auto& t : s.OrbTrails()) assert(t.owner != level+"#"+id);
    game::LevelScene saved;
    assert(saved.Load(level,r,error));
    saved.ApplyCollected(collected);
    saved.UpdateAuthoredOrbs(.5f,ox,oy,ox,oy);
    assert(saved.OrbTrails().empty());
    // A single runtime particle retains its pre-step quad size and no XML warm-up.
    game::ParticleSpray one;
    one.texture=1; one.warmed=true; one.scalesByLifetime=true; one.scaleTo=.1f;
    one.fadeIn=.15f; one.fadeOut=.8f;
    game::ParticleSpray::Particle p;
    p.x=p.startX=p.drawX=ox; p.y=p.startY=p.drawY=oy;
    p.size=p.startSize=p.drawSize=80; p.lifetime=.35f; p.brightness=.8f;
    p.directionX=1; p.speed=p.startSpeed=2;
    one.particles.push_back(p);
    one.Update(1.f/60.f);
    r.drawn.clear(); one.Draw(r,ox,oy);
    assert(r.drawn.size()==1 && std::abs(r.drawn[0].destination.width-80.f*2.f/3.f)<.01f);
    assert(std::abs(one.particles[0].elapsed-1.f/60.f)<.00001f);
    std::cout << "PASS: " << argv[1] << " hover, strict acquire/pickup, trail render and drain\n";
}
