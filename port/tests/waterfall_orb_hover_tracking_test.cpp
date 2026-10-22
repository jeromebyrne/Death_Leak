#include "../game/LevelScene.h"
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
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
int main() {
    setenv("DEATHLEAK_RENDER_ONLY_XML_IDS", "75", 1);
    const std::string level = "Game/XmlFiles/levels/waterfall_top.xml";
    CaptureRenderer r, r2;
    game::LevelScene scene, duplicate;
    std::string error;
    assert(scene.Load(level, r, error) && duplicate.Load(level, r2, error));
    constexpr float ox=-1335.014648f, oy=2995.f;
    auto xy = [&](game::LevelScene& s, CaptureRenderer& renderer) {
        renderer.drawn.clear(); s.Draw(renderer, ox, oy, 9000, 9000);
        assert(renderer.drawn.size()==1);
        const auto& d=renderer.drawn.front();
        assert(d.tint.a==191 || d.tint.a==192);
        return std::pair<float,float>{ox+(d.destination.x+d.destination.width*.5f-640.f)*1.5f,
            oy-(d.destination.y+d.destination.height*.5f-360.f)*1.5f};
    };
    auto initial=xy(scene,r);
    assert(std::abs(initial.first-ox)<.1f && std::abs(initial.second-oy)<.1f);
    scene.UpdateAuthoredOrbs(.25f,9000,9000,ox,oy);
    duplicate.UpdateAuthoredOrbs(.25f,9000,9000,ox,oy);
    auto hover=xy(scene,r);
    auto repeat=xy(duplicate,r2);
    assert(std::abs(hover.second-repeat.second)<.01f);
    assert(std::abs(hover.second-oy)<=20.01f && std::abs(hover.first-ox)<.1f);
    scene.UpdateAuthoredOrbs(1.f,9000,9000,ox+2100,oy);
    assert(std::abs(xy(scene,r).second-hover.second)<.01f); // frozen off-zone
    scene.UpdateAuthoredOrbs(.1f,9000,9000,ox,oy);
    auto moved=xy(scene,r);
    assert(std::abs(moved.second-hover.second)>.01f);
    // Source triggers strictly inside 130 after hover; first tracking tick
    // retains the current position and next tick begins prior-step steering.
    scene.UpdateAuthoredOrbs(0.f,ox+110,moved.second,ox,oy);
    auto armed=xy(scene,r);
    scene.UpdateAuthoredOrbs(0.f,ox+110,moved.second,ox,oy);
    assert(std::abs(xy(scene,r).second-armed.second)<.01f);
    scene.UpdateAuthoredOrbs(1.f/60.f,ox+110,moved.second,ox,oy);
    assert(std::abs(xy(scene,r).first-ox)>1.f);
    std::unordered_set<std::string> collected;
    assert(scene.CollectCurrencyOrbs(9000,9000,collected)==0);
    auto current=xy(scene,r);
    assert(scene.CollectCurrencyOrbs(current.first,current.second,collected)==1);
    assert(scene.CollectCurrencyOrbs(current.first,current.second,collected)==0);
    assert(collected.count(level+"#75")==1);
    scene.UpdateAuthoredOrbs(1.f,current.first,current.second,ox,oy);
    r.drawn.clear();scene.Draw(r,ox,oy,9000,9000);assert(r.drawn.empty());
    duplicate.ApplyCollected(collected);
    r2.drawn.clear();duplicate.Draw(r2,ox,oy,9000,9000);assert(r2.drawn.empty());
    std::cout << "PASS: waterfall authored orb hover, zone, tracking and persistence\n";
}
