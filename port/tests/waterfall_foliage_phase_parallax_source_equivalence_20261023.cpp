// Source-equivalent draw-command gate for waterfall-top ordinary foliage.
#include "../game/LevelScene.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
using namespace deathleak;
namespace {
struct Spy : render::Renderer {
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
struct Case { const char* id; float x,y,w,h,lx,ly,cx; };
constexpr Case cases[] = {
    {"64",-1179.014648f,3660.f,2048.f,677.f,1.10f,1.05f,0.f},
    {"65",2059.985352f,3706.f,2048.f,677.f,1.10f,1.05f,1000.f},
    {"60",-2063.014648f,3571.f,756.f,677.f,1.f,1.f,-1500.f},
};
bool close(float a,float b,float tol=.03f) { return std::fabs(a-b) <= tol; }
bool check(bool b,const char* id,const char* label) {
    if (!b) std::cerr << "FAIL ID" << id << ' ' << label << '\n';
    return b;
}
}
int main(int argc,char** argv) {
    if (argc != 2) { std::cerr << "usage: test 64|65|60\n"; return 2; }
    const Case* c=nullptr;
    for (const auto& v: cases) if (std::string(argv[1])==v.id) c=&v;
    if (!c) return 2;
    setenv("DEATHLEAK_RENDER_ONLY_XML_IDS",c->id,1);
    Spy spy; game::LevelScene scene; std::string error;
    if (!scene.Load("Game/XmlFiles/levels/waterfall_top.xml",spy,error)) {
        std::cerr << error << '\n'; return 2;
    }
    bool ok=true;
    for (int camera=0; camera<2; ++camera) {
        const float cx=c->cx+200.f*camera, cy=3000.f+200.f*camera;
        render::Sprite prior;
        for (int ti=0; ti<2; ++ti) {
            const float time=ti*.5f;
            spy.calls.clear();
            scene.Draw(spy,cx,cy,cx,cy,{},1.f,1.f,{time,0.f});
            if (!check(spy.calls.size()==1,c->id,"one visible XML sprite")) { ok=false; continue; }
            const auto& s=spy.calls.front();
            // GameObject::Update World = authored position - (camera-position)*(depth-1).
            const float wx=c->x-(cx-c->x)*(c->lx-1.f);
            const float wy=c->y-(cy-c->y)*(c->ly-1.f);
            // Existing destination is a separate screen-projection contract and must not move.
            const float dx=640.f+((c->x-cx)*c->lx-c->w/2.f)*2.f/3.f;
            const float dy=360.f+((cy-c->y)*c->ly-c->h/2.f)*2.f/3.f;
            ok &= check(s.foliageSway.enabled && close(s.foliageSway.worldX,wx) &&
                close(s.foliageSway.worldY,wy),c->id,"source World centre");
            ok &= check(close(s.destination.x,dx) && close(s.destination.y,dy) &&
                close(s.destination.width,c->w*2.f/3.f) &&
                close(s.destination.height,c->h*2.f/3.f),c->id,"unchanged placement");
            // Shader takes transformed source-bottom vertex before the sine.
            const float expectedPhase=.1f*(wy-c->h/2.f)+time;
            const float submittedPhase=.1f*(s.foliageSway.worldY-c->h/2.f)+s.foliageSway.time;
            ok &= check(close(submittedPhase,expectedPhase,.003f) &&
                close(20.f*std::sin(submittedPhase),20.f*std::sin(expectedPhase),.01f),
                c->id,"bottom-corner phase and X displacement");
            ok &= check(close(s.foliageSway.bottomLeft.x,20.f) &&
                close(s.foliageSway.bottomRight.x,20.f) &&
                close(s.foliageSway.topLeft.x,0.f) &&
                close(s.foliageSway.globalTimeMultiplier,1.f),c->id,"authored intensity/amplitude");
            if (ti) ok &= check(close(s.destination.x,prior.destination.x) &&
                close(s.destination.y,prior.destination.y) &&
                close(s.foliageSway.worldX,prior.foliageSway.worldX) &&
                close(s.foliageSway.worldY,prior.foliageSway.worldY) &&
                close(s.foliageSway.time-prior.foliageSway.time,.5f),
                c->id,"held-camera time affects phase only");
            else prior=s;
        }
    }
    if (std::string(c->id)=="60") ok &= check(c->lx==1.f && c->ly==1.f,c->id,"unit-depth control");
    std::cout << "WATERFALL_FOLIAGE_SOURCE_EQUIVALENCE=" << (ok?"PASS":"FAIL")
              << " ID=" << c->id << '\n';
    return ok?0:1;
}
