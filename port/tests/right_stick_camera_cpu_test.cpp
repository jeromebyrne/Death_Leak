#include "../game/LevelScene.h"
#include "../game/PlayerMotion.h"
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <string>
#include <unordered_map>
#include <vector>

using namespace deathleak;
static bool near(float a, float b) { return std::fabs(a-b) < 0.001f; }
class CameraSpy final : public render::Renderer {
public:
    std::unordered_map<render::TextureId,std::string> paths;
    std::vector<render::Sprite> draws;
    bool Initialize(const char*, int, int, std::string&) override { return true; }
    render::TextureId LoadTexture(const std::string& path, std::string&) override {
        const auto id = static_cast<render::TextureId>(paths.size()+1);
        paths[id] = path; return id;
    }
    void BeginFrame(render::Color) override {}
    void DrawSprite(const render::Sprite& s) override { draws.push_back(s); }
    void FillRect(render::Rect, render::Color) override {}
    void ApplyScenePostprocess() override {}
    void EndFrame() override {}
    bool PollQuit() override { return false; }
    bool KeyDown(int) const override { return false; }
    render::MouseState Mouse() const override { return {}; }
    bool SaveFrame(const std::string&, std::string&) override { return false; }
};
int main() {
    using game::RightStickCameraTarget;
    using game::FollowCameraWithTarget;
    RightStickCameraTarget t(10.f, -20.f);
    auto update = [&](int rx,int sdlY,bool roll=false,bool fall=false,bool melee=false,
                      bool connected=true,bool eligible=true) {
        t.Update({0,rx,sdlY,connected},eligible,roll,fall,melee);
    };
    update(8689,-8689); assert(near(t.x,10) && near(t.y,-20));
    update(8690,0); assert(near(t.x,10+450.f*8690/32767) && near(t.y,-20));
    update(-9000,0); assert(near(t.x,10-450.f*9000/32767));
    update(0,-9000); assert(near(t.x,10) && near(t.y,-20+300.f*9000/32767));
    update(0,9000); assert(near(t.y,-20-300.f*9000/32767));
    update(9000,-9000); assert(near(t.x,10+450.f*9000/32767) && near(t.y,-20+300.f*9000/32767));
    const float holdX=t.x;
    update(-9000,9000,true); assert(near(t.x,holdX) && near(t.y,-20-300.f*9000/32767));
    update(-9000,-9000,false,true); assert(near(t.x,holdX) && near(t.y,-20+300.f*9000/32767));
    update(-9000,9000,false,false,true); assert(near(t.x,holdX) && near(t.y,-20-300.f*9000/32767));
    update(0,0,false,false,false,false); assert(near(t.x,holdX)); // disconnect holds
    update(0,0,false,false,false,true,false); assert(near(t.x,holdX)); // paused holds
    update(0,0); assert(near(t.x,10) && near(t.y,-20));
    assert(RightStickCameraTarget::WorldRightY(-32768)==32767);
    update(-32768,0); assert(t.x<10-450); // signed-short negative asymmetry

    // Follow precedes input: frame N sees authored target, N+1 sees new target.
    RightStickCameraTarget delayed(10,-20);
    game::CameraFollowInput f{100,200,1000,500,1.5f,10,5,true,true,false};
    float x,y;
    FollowCameraWithTarget(f,delayed,x,y);
    const float oldX=x, oldY=y;
    assert(near(x,100+(1000+15-100)/15) && near(y,200+(500-30-200)/5));
    delayed.Update({0,9000,-9000,true},true,false,false,false);
    f.cameraX=x; f.cameraY=y;
    FollowCameraWithTarget(f,delayed,x,y);
    assert(near(x,oldX+(1000+delayed.x*1.5f-oldX)/15));
    assert(near(y,oldY+(500+delayed.y*1.5f-oldY)/5));
    f.cameraX=x; f.cameraY=y;
    const float beforeNeutral=x;
    FollowCameraWithTarget(f,delayed,x,y); // neutral frame still follows prior target
    assert(near(x,beforeNeutral+(1000+delayed.x*1.5f-beforeNeutral)/15));
    delayed.Update({0,0,0,true},true,false,false,false);
    f.cameraX=x; f.cameraY=y;
    const float afterNeutral=x;
    FollowCameraWithTarget(f,delayed,x,y); // next frame uses authored offset
    assert(near(x,afterNeutral+(1000+15-afterNeutral)/15));
    f.followX=false; f.cameraX=123; f.cameraY=y;
    FollowCameraWithTarget(f,delayed,x,y); assert(near(x,123));
    f.followX=true; f.enemies=true; f.cameraX=0; f.cameraY=0;
    FollowCameraWithTarget(f,delayed,x,y); assert(near(x,(1000.f+15.f)/60.f));
    f.bounded=true; f.left=0; f.right=2200; f.bottom=0; f.top=1400;
    f.cameraX=0; f.cameraY=0;
    FollowCameraWithTarget(f,delayed,x,y);
    assert(near(x,960.f+(1015.f-960.f)/60.f) && near(y,540)); // pre/post follow clamp

    // Emitted float UV phase and destination share the camera transform.
    setenv("DEATHLEAK_RENDER_ONLY_XML_IDS","37,2,__player_aim_line",1);
    CameraSpy spy; game::LevelScene scene; std::string error;
    assert(scene.Load("Game/XmlFiles/levels/forest_exploration_1.xml",spy,error));
    // Drive the real post-contact motion state into the right-stick gate.
    game::PlayerMotion motion;
    using Recovery = game::PlayerMotion::ImpactRecovery;
    const auto& floor = scene.Ground().front();
    const float landingX = (floor.x1 + floor.x2) * 0.5f;
    const float floorY = (floor.y1 + floor.y2) * 0.5f;
    constexpr float halfPlayerHeight = 214.898743f / 2.f;
    auto landFrom = [&](float drop, bool authored = false) {
        motion.Reset(landingX, floorY + halfPlayerHeight + drop, authored);
        // Single CPU step starts at rest and crosses the selected floor.
        motion.Update(scene, std::sqrt(2.f * (drop + 1.f) / ((0.75f / 0.55f) * 3600.f)), 0, false);
        assert(motion.Grounded());
    };
    landFrom(700.f, true); assert(!motion.ImpactRecovering());
    landFrom(300.f); assert(motion.Recovery() == Recovery::None);
    landFrom(300.125f); assert(motion.Recovery() == Recovery::Soft);
    motion.Update(scene, 1.f/60.f, 0, false);
    assert(motion.Recovery() == Recovery::Soft); // not a one-tick pulse
    RightStickCameraTarget impactTarget(10.f, -20.f);
    impactTarget.Update({0,9000,0,true},true,false,false,false);
    const float impactHoldX = impactTarget.x;
    impactTarget.Update({0,-9000,-9000,true},true,motion.Rolling(),motion.ImpactRecovering(),false);
    assert(near(impactTarget.x,impactHoldX) && near(impactTarget.y,-20.f+300.f*9000.f/32767.f));
    impactTarget.Update({0,0,0,true},true,false,motion.ImpactRecovering(),false);
    assert(near(impactTarget.x,10.f) && near(impactTarget.y,-20.f));
    motion.Update(scene, 0.25f, 0, false);
    assert(!motion.ImpactRecovering());
    impactTarget.Update({0,-9000,0,true},true,false,motion.ImpactRecovering(),false);
    assert(near(impactTarget.x,10.f-450.f*9000.f/32767.f));
    landFrom(699.875f); assert(motion.Recovery() == Recovery::Soft);
    landFrom(700.f); assert(motion.Recovery() == Recovery::Hard);
    motion.Update(scene, 1.f/60.f, 0, false); assert(motion.ImpactRecovering());
    motion.Update(scene, 0.75f, 0, false); assert(!motion.ImpactRecovering());
    landFrom(700.f); assert(motion.Recovery() == Recovery::Hard);
    motion.Reset(landingX, floorY + halfPlayerHeight, true);
    assert(!motion.ImpactRecovering());
    // Airborne descent is not itself recovery; retained dash contact is hard
    // even when the measured drop is below the distance threshold.
    motion.SetMovementUnlocks(true, true);
    motion.Reset(landingX, floorY + halfPlayerHeight + 250.f);
    motion.Update(scene, 0.25f, 0, false);
    assert(!motion.Grounded() && !motion.ImpactRecovering());
    assert(motion.TryDownwardDash(true));
    assert(!motion.ImpactRecovering());
    motion.Update(scene, 0.1f, 0, false);
    assert(motion.Grounded() && motion.Recovery() == Recovery::Hard);
    impactTarget.Update({0,9000,-9000,true},true,false,motion.ImpactRecovering(),false);
    const float hardHoldX = impactTarget.x;
    assert(near(impactTarget.y,-20.f+300.f*9000.f/32767.f));
    impactTarget.Update({0,-9000,9000,true},true,false,motion.ImpactRecovering(),false);
    assert(near(impactTarget.x,hardHoldX) && near(impactTarget.y,-20.f-300.f*9000.f/32767.f));
    // A successful buffered roll after landing clears the recovery immediately.
    motion.SetMovementUnlocks(false, true);
    landFrom(700.f); assert(motion.ImpactRecovering());
    motion.BufferRoll(); assert(motion.TryBufferedRoll(true));
    assert(motion.Rolling() && !motion.ImpactRecovering());

    auto draw = [&](float cx,float cy) {
        spy.draws.clear(); scene.Draw(spy,cx,cy,scene.PlayerX(),scene.PlayerY());
        std::vector<render::Sprite> out;
        for (const auto& s:spy.draws) if (spy.paths.at(s.texture).find("Game/Media/characters/player/") != 0) out.push_back(s);
        return out;
    };
    scene.UpdatePlayerVisual(1.f/60.f,0,0,true,0,false,false,false,0,1,false,true);
    const float cx=scene.PlayerX(), cy=scene.PlayerY();
    const auto a=draw(cx,cy), b=draw(cx+7,cy+11);
    bool parallaxChanged=false, aimMoved=false;
    assert(a.size()==b.size());
    for (size_t i=0;i<a.size();++i) {
        if (spy.paths.at(a[i].texture)!="Game/Media/aim_line.png") {
            if (!near(a[i].destination.y,b[i].destination.y) &&
                !near(a[i].uvShiftX,b[i].uvShiftX) && near(a[i].uvShiftY,b[i].uvShiftY))
                parallaxChanged=true;
        } else if (spy.paths.at(a[i].texture)=="Game/Media/aim_line.png") {
            aimMoved=near(b[i].destination.x-a[i].destination.x,-7.f*2.f/3.f) &&
                     near(b[i].destination.y-a[i].destination.y,11.f*2.f/3.f) &&
                     near(a[i].rotationDegrees,b[i].rotationDegrees);
        }
    }
    assert(parallaxChanged && aimMoved);
}
