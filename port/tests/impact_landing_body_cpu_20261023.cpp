#include "../game/LevelScene.h"
#include "../game/PlayerMotion.h"
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
using namespace deathleak;
class Spy final : public render::Renderer {
public:
    std::unordered_map<render::TextureId,std::string> paths;
    std::vector<render::Sprite> draws;
    render::TextureId LoadTexture(const std::string& path,std::string&) override { auto id=++next; paths[id]=path; return id; }
    bool Initialize(const char*,int,int,std::string&) override { return true; }
    void BeginFrame(render::Color) override {}
    void DrawSprite(const render::Sprite& s) override { draws.push_back(s); }
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
    setenv("DEATHLEAK_RENDER_ONLY_XML_IDS","2",1);
    Spy spy; game::LevelScene scene; std::string error;
    assert(scene.Load("Game/XmlFiles/levels/forest_exploration_1.xml",spy,error));
    game::PlayerMotion motion; motion.SetImpactVisualDriven(true);
    const auto& floor=scene.Ground().front();
    const float x=(floor.x1+floor.x2)*.5f, y=(floor.y1+floor.y2)*.5f;
    const auto land=[&](float drop) {
        motion.Reset(x,y+214.898743f/2.f+drop);
        motion.Update(scene,std::sqrt(2.f*(drop+1.f)/((.75f/.55f)*3600.f)),0,false);
        assert(motion.Grounded());
    };
    const auto body=[&]() {
        spy.draws.clear(); scene.Draw(spy,x,y,x,y);
        std::string result;
        for (const auto& draw:spy.draws) {
            const auto& path=spy.paths.at(draw.texture);
            if (path.find("Game/Media/characters/player/")==0 && path.find("/back_arm_")==std::string::npos)
                result=path;
        }
        assert(!result.empty()); return result;
    };
    const auto tick=[&](float prior=1.f/60.f,float melee=0.f,bool crouch=false) {
        scene.UpdatePlayerVisual(1.f/60.f,0,0,true,0,motion.Rolling(),false,motion.InWater(),melee,1,
                                 false,false,{},false,0,0,prior,crouch,motion.Recovery());
        const auto path=body();
        if (scene.ImpactVisualFinished()) motion.CompleteImpactRecovery();
        return path;
    };
    const std::string duck="Game/Media/characters/player/duck_default/duck_default_00";
    land(300.f); assert(motion.Recovery()==game::PlayerImpactRecovery::None);
    assert(tick(0).find("idle_default")!=std::string::npos);
    land(300.125f); assert(motion.Recovery()==game::PlayerImpactRecovery::Soft);
    assert(tick(0,1.f,true)==duck+"5.png"); // Impact outranks melee/crouch.
    const int soft[]={5,4,3,2,1,0};
    for (int frame=1;frame<6;++frame) {
        assert(tick(1.f/30.f)==duck+std::to_string(soft[frame])+".png");
        assert(motion.ImpactRecovering());
    }
    assert(tick(1.f/30.f)==duck+"0.png");
    assert(!motion.ImpactRecovering());
    assert(tick(0).find("idle_default")!=std::string::npos);
    land(700.f); assert(motion.Recovery()==game::PlayerImpactRecovery::Hard);
    assert(tick(0)==duck+"5.png");
    const int hard[]={5,5,5,5,5,5,4,3,2,1,0};
    for (int frame=1;frame<11;++frame) {
        assert(tick(1.f/15.f)==duck+std::to_string(hard[frame])+".png");
        assert(motion.ImpactRecovering());
    }
    assert(tick(1.f/15.f)==duck+"0.png");
    assert(!motion.ImpactRecovering());
    land(700.f); assert(motion.TryRoll(true)==false); // Locked roll cannot cancel.
    motion.SetMovementUnlocks(false,true);
    assert(motion.TryRoll(true)); assert(!motion.ImpactRecovering());
    assert(tick(0).find("roll_default")!=std::string::npos);
    motion.SetMovementUnlocks(true,true);
    motion.Reset(x,y+214.898743f/2.f+250.f);
    motion.Update(scene,.25f,0,false);
    assert(!motion.Grounded() && motion.TryDownwardDash(true));
    motion.Update(scene,.1f,0,false);
    assert(motion.Grounded() && motion.Recovery()==game::PlayerImpactRecovery::Hard);
    assert(tick(0)==duck+"5.png"); // Short downward dash still selects hard pose.
    motion.BufferRoll(); assert(motion.TryBufferedRoll(true));
    assert(!motion.ImpactRecovering());
    assert(tick(0).find("roll_default")!=std::string::npos);
    motion.Reset(x,y+214.898743f/2.f+700.f);
    motion.SetMovementUnlocks(false,true);
    motion.Update(scene,std::sqrt(2.f*701.f/((.75f/.55f)*3600.f)),0,false);
    assert(motion.Recovery()==game::PlayerImpactRecovery::Hard);
    assert(motion.TryRoll(true));
    // A roll already active at contact cannot acquire distance recovery.
    std::cout<<"PASS: soft/hard authored texture order, held frames, completion, pose priority, 300 threshold, short dash and roll cancellation\n";
}
