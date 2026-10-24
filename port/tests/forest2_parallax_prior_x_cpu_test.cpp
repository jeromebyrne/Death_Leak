// Forest-2 production draw-spy: Windows GameObject world transform is captured
// before ParallaxLayer stores the camera position for the next eligible update.
#include "../game/LevelScene.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>
using namespace deathleak;
struct Spy final : render::Renderer {
    std::vector<render::Sprite> calls;
    bool Initialize(const char*, int, int, std::string&) override { return true; }
    render::TextureId LoadTexture(const std::string&, std::string&) override { return 1; }
    void BeginFrame(render::Color) override {}
    void DrawSprite(const render::Sprite& s) override { calls.push_back(s); }
    void FillRect(render::Rect, render::Color) override {}
    void ApplyScenePostprocess() override {}
    void EndFrame() override {}
    bool PollQuit() override { return false; }
    bool KeyDown(int) const override { return false; }
    render::MouseState Mouse() const override { return {}; }
    bool SaveFrame(const std::string&, std::string&) override { return false; }
};
static void near(float actual,float expected,const char* label) {
    if (std::fabs(actual-expected)>.005f) {
        std::cerr << "FAIL " << label << ": " << actual << " != " << expected << '\n'; std::exit(1);
    }
}
int main() {
    setenv("DEATHLEAK_RENDER_ONLY_XML_IDS","1,28,36",1);
    Spy spy; game::LevelScene scene; std::string error;
    if (!scene.Load("Game/XmlFiles/levels/forest_exploration_2.xml",spy,error)) {
        std::cerr << error << '\n'; return 1;
    }
    constexpr float authored=1817.985352f, y=3400.f, scale=2.f/3.f;
    const float widths[]={1985.f,2048.f,2048.f};
    const float heights[]={1090.f,677.f,512.f};
    const float multipliers[]={.000059f,.000690f,.000690f};
    auto draw=[&](float camera,float previousFollow) {
        spy.calls.clear(); scene.Draw(spy,camera,y,scene.PlayerX(),scene.PlayerY());
        if(spy.calls.size()!=3) { std::cerr << "FAIL command count " << spy.calls.size() << '\n'; std::exit(1); }
        for(int i=0;i<3;++i) {
            const auto& s=spy.calls[i];
            near(s.destination.x,640.f+(previousFollow-camera-widths[i]/2.f)*scale,"prior-world destination");
            near(s.destination.width,widths[i]*scale,"width");
            near(s.destination.height,heights[i]*scale,"height");
            near(s.uvShiftX,camera*multipliers[i],"current-camera UV");
        }
    };
    auto update=[&](float camera,bool paused=false) {
        scene.UpdateVisuals(1.f/60.f,scene.PlayerX(),scene.PlayerY(),camera,y,paused);
    };
    update(100.f); draw(100.f,authored); // first update uses authored matrix
    update(108.f); draw(108.f,100.f);   // second uses previous followed position
    update(108.f); draw(108.f,108.f);   // held camera catches up
    update(116.f); update(124.f); draw(124.f,116.f); // two updates, one draw
    update(132.f,true); draw(132.f,116.f); draw(132.f,116.f); // paused/repeated draw
    std::cout << "PASS Forest-2 IDs 1/28/36 prior eligible world X and current UV\n";
}
