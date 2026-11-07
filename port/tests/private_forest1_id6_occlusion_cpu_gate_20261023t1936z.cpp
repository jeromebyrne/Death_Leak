// CPU-only production LevelScene ID6 occlusion draw-command spy.
#include "../game/LevelScene.h"
#include "../../Game/tinyxml/tinyxml.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
namespace render = deathleak::render;
struct Spy final : render::Renderer {
    struct Command { std::string path; render::Sprite sprite; };
    std::unordered_map<std::string, render::TextureId> ids;
    std::unordered_map<render::TextureId, std::string> paths;
    std::vector<Command> commands;
    bool Initialize(const char*, int, int, std::string&) override { return true; }
    render::TextureId LoadTexture(const std::string& path, std::string&) override {
        auto [it, inserted] = ids.try_emplace(path, static_cast<render::TextureId>(ids.size()+1));
        paths[it->second] = path; return it->second;
    }
    void BeginFrame(render::Color) override {}
    void DrawSprite(const render::Sprite& sprite) override { commands.push_back({paths.at(sprite.texture), sprite}); }
    void FillRect(render::Rect, render::Color) override {}
    void ApplyScenePostprocess() override {}
    void EndFrame() override {}
    bool PollQuit() override { return false; }
    bool KeyDown(int) const override { return false; }
    render::MouseState Mouse() const override { return {}; }
    bool SaveFrame(const std::string&, std::string&) override { return false; }
};
static void Check(bool ok, const std::string& what) {
    if (!ok) { std::cerr << "FAIL: " << what << '\n'; std::exit(1); }
}
static bool Near(float a,float b,float tolerance=2e-4f) { return std::fabs(a-b)<tolerance; }
int main() {
    constexpr char level[]="Game/XmlFiles/levels/forest_exploration_1.xml";
    TiXmlDocument doc(level); Check(doc.LoadFile(),"XML load");
    const TiXmlElement *id6=nullptr,*id10=nullptr;
    for (auto* e=doc.RootElement()->FirstChildElement();e;e=e->NextSiblingElement()) {
        if (!e->Attribute("id")) continue;
        if (std::string(e->Attribute("id"))=="6") id6=e;
        if (std::string(e->Attribute("id"))=="10") id10=e;
    }
    Check(id6 && id10,"XML IDs 6 and 10");
    auto* pos=id6->FirstChildElement("position");
    auto* dim=id6->FirstChildElement("dimensions");
    auto* fade=id6->FirstChildElement("fade_when_player_occluded");
    Check(std::string(pos->Attribute("depth_layer"))=="kGroundFront","ID6 depth");
    Check(std::string(fade->Attribute("value"))=="true","ID6 fade enabled");
    Check(std::string(id10->FirstChildElement("fade_when_player_occluded")->Attribute("value"))=="false","ID10 fade disabled");
    float x=std::stof(pos->Attribute("x")),y=std::stof(pos->Attribute("y"));
    float width=std::stof(dim->Attribute("width")),height=std::stof(dim->Attribute("height"));
    Check(Near(x,1339.985352f)&&Near(y,3895.f)&&Near(width,1112.317383f)&&Near(height,2743.384033f),"ID6 authored geometry");
    Check(Near(std::stof(fade->Attribute("alphawhenoccluding")),.4f),"ID6 occluded endpoint");
    setenv("DEATHLEAK_RENDER_ONLY_XML_IDS","2,6,10",1);
    Spy spy; deathleak::game::LevelScene scene; std::string error;
    Check(scene.Load(level,spy,error),"LevelScene load: "+error);
    constexpr float dt=1.f/60.f;
    // Render around ID6 throughout, including after a remote camera update.
    auto draw=[&](const char* state,float expected) {
        spy.commands.clear(); scene.Draw(spy,x,y,x,y);
        size_t body=spy.commands.size(),tree=spy.commands.size(),blur=spy.commands.size();
        for(size_t i=0;i<spy.commands.size();++i) {
            const auto& c=spy.commands[i];
            if(c.path.find("Game/Media/characters/player/idle_default/idle_default_")==0) body=i;
            // XML filter excludes ID5. Also match ID6's authored world-quad geometry,
            // so tree2.png alone can never identify this command.
            if(c.path=="Game/Media/objects/tree2.png" &&
               Near(c.sprite.destination.width,width*2.f/3.f) &&
               Near(c.sprite.destination.height,height*2.f/3.f)) tree=i;
            if(c.path=="Game/Media/objects/tree2_blur.png") blur=i;
        }
        Check(body<tree && tree<blur && blur<spy.commands.size(),std::string(state)+" painter order ID2>ID6>ID10");
        const auto& c=spy.commands[tree].sprite;
        Check(Near(c.opacity,expected),std::string(state)+" ID6 opacity expected "+std::to_string(expected)+" actual "+std::to_string(c.opacity));
        Check(c.tint.a==std::lround(std::clamp(expected,0.f,1.f)*255.f),std::string(state)+" ID6 tint alpha");
        Check(c.flipX && c.flipY,std::string(state)+" ID6 authored flip/SDL V convention");
        const auto& control=spy.commands[blur].sprite;
        Check(Near(control.opacity,.7f)&&control.tint.a==179,std::string(state)+" ID10 nonfade control");
        std::cout<<state<<" ID6 opacity="<<c.opacity<<" tint.a="<<int(c.tint.a)<<"; ID10="<<control.opacity<<"/"<<int(control.tint.a)<<'\n';
    };
    // Left edge itself is outside the strict source rectangle.
    const float edge=x-width*.5f;
    scene.UpdateVisuals(dt,edge,y,x,y); draw("outside-edge",1.f);
    scene.UpdateVisuals(dt,x,y,x,y); draw("inside-1",.975f);
    for(int n=2;n<=24;++n) scene.UpdateVisuals(dt,x,y,x,y);
    draw("inside-24",.4f); // Approximately 0.4; float roundoff permits one more source step.
    scene.UpdateVisuals(dt,x,y,x,y); draw("inside-25-overshoot",.375f);
    scene.UpdateVisuals(dt,x,y,x,y); draw("inside-clamp",.4f);
    scene.UpdateVisuals(dt,edge,y,x,y); draw("exit-1",.425f);
    for(int n=2;n<=23;++n) scene.UpdateVisuals(dt,edge,y,x,y);
    draw("exit-23",.975f);
    scene.UpdateVisuals(dt,edge,y,x,y); draw("exit-24",1.f);
    scene.UpdateVisuals(dt,edge,y,x,y); draw("exit-25-overshoot",1.025f);
    scene.UpdateVisuals(dt,edge,y,x,y); draw("exit-clamp",1.f);
    // ID6's bounds do not intersect a 4000x4000 update zone at this camera.
    scene.UpdateVisuals(dt,x,y,10000.f,10000.f); draw("camera-outside-freeze",1.f);
    scene.UpdateVisuals(dt,x,y,x,y); draw("camera-return-inside",.975f);
    std::cout<<"PASS: production Forest-1 ID6 CPU draw-command fade/strict-boundary/order/freeze contract\n";
}
