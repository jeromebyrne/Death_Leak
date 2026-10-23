#include "../game/PlayerMotion.h"
#include <cassert>
#include <cmath>
#include <string>
#include <unordered_map>
using namespace deathleak;
class StubRenderer final : public render::Renderer {
public:
    render::TextureId LoadTexture(const std::string&,std::string&) override { return ++next; }
    bool Initialize(const char*,int,int,std::string&) override { return true; }
    void BeginFrame(render::Color) override {}
    void DrawSprite(const render::Sprite&) override {}
    void FillRect(render::Rect,render::Color) override {}
    void ApplyScenePostprocess() override {}
    void EndFrame() override {}
    bool PollQuit() override { return false; }
    bool KeyDown(int) const override { return false; }
    render::MouseState Mouse() const override { return {}; }
    bool SaveFrame(const std::string&,std::string&) override { return false; }
private: render::TextureId next=0;
};
int main() {
    StubRenderer renderer; game::LevelScene scene; std::string error;
    assert(scene.Load("Game/XmlFiles/levels/forest_exploration_1.xml",renderer,error));
    game::PlayerMotion motion;
    using Event=game::PlayerMotion::HardLandingEvent;
    const auto& floor=scene.Ground().front();
    const float x=(floor.x1+floor.x2)*0.5f, y=(floor.y1+floor.y2)*0.5f+214.898743f/2.f;
    auto land=[&](float drop,bool authored=false) {
        motion.Reset(x,y+drop,authored);
        motion.Update(scene,std::sqrt(2.f*(drop+1.f)/((0.75f/0.55f)*3600.f)),0,false);
        assert(motion.Grounded());
    };
    land(700.f,true); assert(motion.ConsumeHardLandingEvent()==Event::None);
    land(699.875f); assert(motion.ConsumeHardLandingEvent()==Event::None);
    land(700.f); assert(motion.ConsumeHardLandingEvent()==Event::LargeDrop);
    assert(motion.ConsumeHardLandingEvent()==Event::None);
    motion.Update(scene,1.f/60.f,0,false);
    assert(motion.ConsumeHardLandingEvent()==Event::None);
    motion.SetMovementUnlocks(false,true);
    land(700.f); motion.BufferRoll(); assert(motion.TryBufferedRoll(true));
    assert(motion.ConsumeHardLandingEvent()==Event::LargeDropRoll);
    assert(motion.ConsumeHardLandingEvent()==Event::None);
    motion.SetMovementUnlocks(true,true);
    motion.Reset(x,y+250.f);
    motion.Update(scene,0.25f,0,false); assert(motion.TryDownwardDash(true));
    motion.Update(scene,0.1f,0,false); assert(motion.Grounded());
    assert(motion.ConsumeHardLandingEvent()==Event::DownwardDash);
    assert(motion.ConsumeHardLandingEvent()==Event::None);
}
