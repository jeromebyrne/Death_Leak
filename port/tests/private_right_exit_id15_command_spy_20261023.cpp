// CPU-only production LevelScene command oracle at an illustrative right-exit camera.
// Run from the repository source root; no SDL/backend is linked.
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
    void DrawSprite(const render::Sprite& sprite) override { commands.push_back({paths.at(sprite.texture),sprite}); }
    void FillRect(render::Rect, render::Color) override {}
    void ApplyScenePostprocess() override {}
    void EndFrame() override {}
    bool PollQuit() override { return false; }
    bool KeyDown(int) const override { return false; }
    render::MouseState Mouse() const override { return {}; }
    bool SaveFrame(const std::string&, std::string&) override { return false; }
};
static void Check(bool yes, const std::string& what) {
    if (!yes) { std::cerr << "FAIL: " << what << '\n'; std::exit(1); }
}
static bool Near(float a, float b, float tolerance=.025f) { return std::fabs(a-b) <= tolerance; }
struct Expected {
    const char* id; const char* path; const char* depth;
    float x,y,w,h,rotation,opacity,uvX;
    unsigned alpha; bool flipX,flipY,repeatX;
};
int main() {
    constexpr char level[]="Game/XmlFiles/levels/forest_exploration_1.xml";
    constexpr float C=2360.52f,D=3400.f,playerX=2360.52f,playerY=3114.74f,dt=1.f/60.f;
    const std::vector<Expected> expected = {
        {"19","Game/Media/levels/waterfall_top/ground_blur.png","kMiddleBackground",-1740.145f-C/3.f,360.f+((D-2808.099121f)*.97f-1024.f/2.f)*2.f/3.f,4902.746f,682.667f,0,1,0,255,true,true,true},
        {"21","Game/Media/objects/tree_strip_dark.png","kMiddleBackground",-247.972f-C/3.f,360.f+((D-3736.f)*.97f-1238.f/2.f)*2.f/3.f,1323.333f,825.333f,0,1,0,255,false,true,false},
        {"38","Game/Media/blank_white.png","kMiddleBackground",-5920.338f-C/3.f,360.f+((D-3586.f)*.97f-2743.384033f/2.f)*2.f/3.f,13333.333f,1828.923f,0,.15f,0,38,true,true,false},
        {"17","Game/Media/levels/waterfall_top/ground.png","kNearBackground",-1701.881f-C/2.f,360.f+((D-2724.f)*.99f-1024.f/2.f)*2.f/3.f,4902.746f,682.667f,0,1,0,255,true,true,true},
        {"22","Game/Media/objects/tree_strip.png","kNearBackground",-986.877f-C/2.f,360.f+((D-3572.f)*.99f-1238.f/2.f)*2.f/3.f,3364.740f,825.333f,0,1,0,255,true,true,true},
        {"39","Game/Media/blank_white.png","kGroundBack",-5897.343f-2*C/3.f,360.f+((D-3415.f)-2743.384033f/2.f)*2.f/3.f,13333.333f,1828.923f,0,.20f,0,51,true,true,false},
        {"15","Game/Media/sun_shaft_2.png","kGroundFront",2333.990f-2*C/3.f,360.f+((D-3207.f)-512.f/2.f)*2.f/3.f,341.333f,341.333f,84.22474f,1,0,255,false,true,false},
        {"37","Game/Media/levels/forest_exploration_1/shrubbery.png","kMiddleForeground",-42.667f,360.f+((D-2800.f)-512.f/2.f)*2.f/3.f,1365.333f,341.333f,0,.95f,.000690f*C,242,false,true,false}
    };
    TiXmlDocument doc(level); Check(doc.LoadFile(),"shipped Forest-1 XML load");
    for (const auto& e:expected) {
        const TiXmlElement* found=nullptr; int count=0;
        for (auto* node=doc.RootElement()->FirstChildElement();node;node=node->NextSiblingElement())
            if (node->Attribute("id") && e.id==std::string(node->Attribute("id"))) { found=node; ++count; }
        Check(count==1,"unique XML ID "+std::string(e.id));
        Check(std::string(found->FirstChildElement("position")->Attribute("depth_layer"))==e.depth,"XML depth ID "+std::string(e.id));
        Check(std::string(found->FirstChildElement("effect")->Attribute("name"))=="effectlighttexture","XML effect ID "+std::string(e.id));
    }
    setenv("DEATHLEAK_RENDER_ONLY_XML_IDS","15,17,19,21,22,37,38,39",1);
    unsetenv("DEATHLEAK_RENDER_HIDE_XML_IDS");
    Spy spy; deathleak::game::LevelScene scene; std::string error;
    Check(scene.Load(level,spy,error),"production LevelScene load: "+error);
    scene.UpdateVisuals(dt,playerX,playerY,C,D);
    scene.Draw(spy,C,D,playerX,playerY);
    Check(spy.commands.size()==expected.size(),"exactly eight admitted submitted commands, got "+std::to_string(spy.commands.size()));
    for (size_t i=0;i<expected.size();++i) {
        const auto& e=expected[i]; const auto& c=spy.commands[i]; const auto& s=c.sprite;
        const std::string label="ID "+std::string(e.id)+" ordinal "+std::to_string(i);
        Check(c.path==e.path,label+" texture/painter order: "+c.path);
        Check(Near(s.destination.x,e.x)&&Near(s.destination.y,e.y)&&Near(s.destination.width,e.w)&&Near(s.destination.height,e.h),label+" destination actual=("+std::to_string(s.destination.x)+","+std::to_string(s.destination.y)+","+std::to_string(s.destination.width)+","+std::to_string(s.destination.height)+") expected=("+std::to_string(e.x)+","+std::to_string(e.y)+","+std::to_string(e.w)+","+std::to_string(e.h)+")");
        Check(Near(s.source.x,0)&&Near(s.source.y,0)&&Near(s.source.width,0)&&Near(s.source.height,0),label+" full-texture source");
        Check(Near(s.rotationDegrees,e.rotation,.002f),label+" rotation");
        Check(s.flipX==e.flipX&&s.flipY==e.flipY&&s.repeatX==e.repeatX,label+" flips/repeat");
        Check(s.tint.r==255&&s.tint.g==255&&s.tint.b==255&&s.tint.a==e.alpha,label+" tint");
        Check(Near(s.opacity,e.opacity,.0001f),label+" opacity");
        Check(Near(s.uvShiftX,e.uvX,.0002f)&&Near(s.uvShiftY,0,.0002f),label+" UV shift");
        Check(!s.foliageSway.enabled&&!s.pixelWobble.enabled&&Near(s.rgbGain,1),label+" effect flags/RGB gain");
        std::cout << e.id << " " << c.path << " dest=(" << s.destination.x << "," << s.destination.y << "," << s.destination.width << "," << s.destination.height << ") rotation=" << s.rotationDegrees << " alpha=" << int(s.tint.a) << " uvX=" << s.uvShiftX << '\n';
    }
    std::cout << "PASS: Forest-1 production CPU submission at illustrative C=2360.52,D=3400; ID15 admitted once at ordinal 6/8\n";
}
