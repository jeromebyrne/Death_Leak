#include "../game/PlayerMotion.h"
#include "../audio/SoundPlayer.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <string>
#include <vector>
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
class SpySound final : public audio::SoundPlayer {
public:
    bool Initialize(std::string&) override { return true; }
    void Play(audio::Cue cue, int variation) override { assert(cue == audio::Cue::JumpVocal); played.push_back(variation); }
    unsigned PlayDevilVoice(bool) override { return 0; }
    bool VoiceFinished(unsigned) override { return true; }
    void SetSfxEnabled(bool) override {}
    void SetAmbience(const std::vector<audio::AmbientSource>&) override {}
    void UpdateAmbience(float,float) override {}
    std::vector<int> played;
};
int main() {
    StubRenderer renderer; game::LevelScene scene; std::string error;
    assert(scene.Load("Game/XmlFiles/levels/forest_exploration_1.xml", renderer, error));
    game::PlayerMotion motion; SpySound sound;
    const auto& floor = scene.Ground().front();
    const float x = (floor.x1+floor.x2)*0.5f, y = (floor.y1+floor.y2)*0.5f+214.898743f/2.f;
    auto grounded = [&] { motion.Reset(x,y,true); motion.Update(scene,1.f/60.f,0,false); assert(motion.Grounded()); assert(motion.ConsumeJumpRollVocalActions()==0); };
    grounded();
    motion.Update(scene,1.f/60.f,0,true);
    assert(motion.ConsumeJumpRollVocalActions()==1);
    for (int i=0;i<20;++i) { motion.Update(scene,1.f/60.f,0,true); assert(motion.ConsumeJumpRollVocalActions()==0); }
    motion.Update(scene,1.f/60.f,0,false);
    motion.Update(scene,1.f/60.f,0,true); // already airborne and outside coyote window
    assert(motion.ConsumeJumpRollVocalActions()==0);
    assert(!motion.TryRoll(true)); assert(motion.ConsumeJumpRollVocalActions()==0);
    grounded();
    motion.SetMovementUnlocks(false,true);
    assert(!motion.TryRoll(false)); assert(motion.ConsumeJumpRollVocalActions()==0);
    assert(motion.TryRoll(true)); assert(motion.ConsumeJumpRollVocalActions()==1);
    assert(!motion.TryRoll(true)); assert(motion.ConsumeJumpRollVocalActions()==0);
    motion.Reset(x,y+100.f);
    motion.SetMovementUnlocks(false,true); motion.BufferRoll();
    motion.Update(scene,std::sqrt(2.f*101.f/((0.75f/0.55f)*3600.f)),0,false);
    assert(motion.Grounded()); assert(motion.TryBufferedRoll(true));
    assert(motion.ConsumeJumpRollVocalActions()==1); assert(motion.ConsumeJumpRollVocalActions()==0);
    // A wall jump is the original's separate jump.wav action, not PlayJumpSFX.
    game::LevelScene wallScene;
    constexpr float wallX = 100080.f, wallY = 100000.f;
    auto& wallGround = const_cast<std::vector<game::GroundSegment>&>(wallScene.Ground());
    wallGround.push_back({wallX, wallY-2000.f, wallX, wallY+2000.f, "stone"});
    motion.Reset(100000.f, wallY);
    for (int i=0; i<25 && motion.X()<wallX-64.48f; ++i)
        motion.Update(wallScene,1.f/60.f,1,false);
    assert(std::abs(motion.X()-(wallX-128.959793f/2.f))<0.1f);
    assert(motion.ConsumeJumpRollVocalActions()==0);
    motion.Update(wallScene,1.f/60.f,0,true);
    assert(motion.X()<wallX-128.959793f/2.f); // wall-jump impulse really fired
    assert(motion.ConsumeJumpRollVocalActions()==0);
    // Every authored branch, and both boundaries of the silent region.
    for (int draw=0;draw<20;++draw) audio::DispatchJumpRollVocal(sound,draw);
    assert((sound.played==std::vector<int>{0,1,2,3,4}));
    // In water, held jump is a swim action, never a jump vocal.
    game::LevelScene waterScene;
    assert(waterScene.Load("Game/XmlFiles/levels/underwater_cave.xml", renderer, error));
    const auto water = std::find_if(waterScene.Hazards().begin(),waterScene.Hazards().end(),
        [](const auto& hazard) { return hazard.water; });
    assert(water != waterScene.Hazards().end());
    {
        motion.Reset(water->x,water->y);
        motion.Update(waterScene,1.f/60.f,0,true);
        assert(motion.ConsumeJumpRollVocalActions()==0);
        assert(!motion.TryRoll(true));
    }
}
