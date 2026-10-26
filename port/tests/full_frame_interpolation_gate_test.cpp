#include "../linux/FullFrameInterpolationGate.h"
#include "../linux/RealSceneTopologyCapture.h"
#include "../linux/FixedStepPacer.h"
#include <cassert>
#include <chrono>
#include <cstdlib>
#include <iostream>
using namespace deathleak;
class Spy final : public render::Renderer {
public:
 bool Initialize(const char*,int,int,bool,std::string&) override { return true; }
 render::TextureId LoadTexture(const std::string&,std::string&) override { return 1; }
 void BeginFrame(render::Color) override {} void DrawSprite(const render::Sprite&) override {}
 void FillRect(render::Rect,render::Color) override {} void ApplyScenePostprocess() override {}
 void EndFrame() override {} bool PollQuit() override { return false; }
 bool KeyDown(int) const override { return false; } render::MouseState Mouse() const override { return {}; }
 bool SaveFrame(const std::string&,std::string&) override { return false; }
};
static linux_port::FullFrameSnapshot Capture(game::LevelScene& scene,
        linux_port::PresentationRecordingRenderer& recorder,float cameraX,float cameraY) {
 recorder.BeginFrame({0,0,0,255});
 scene.Draw(recorder,cameraX,cameraY,scene.PlayerX(),scene.PlayerY(),{},1.f,1.f,{},false,{}, {},
   [&](const game::SceneSprite& o,const render::Sprite&,bool arm) {
     // Distinct body/arm subparts; scene epoch is tracked outside this key.
     assert(recorder.TagLastSprite({(o.stableIdentity<<1) | unsigned(arm),0},
       o.parallaxLayer ? 0.f : linux_port::OrdinaryCameraFactorX(o.depth),
       o.parallaxLayer ? 0.f : linux_port::OrdinaryCameraFactorY(o.depth),
       o.parallaxLayer && !o.autoScrollY ? linux_port::PresentationCommandPacket::PhasePolicy::CameraParallax :
       o.parallaxLayer ? linux_port::PresentationCommandPacket::PhasePolicy::AutoScrollY :
       (o.foliageSway.enabled || o.pixelWobble.enabled) ? linux_port::PresentationCommandPacket::PhasePolicy::ShaderClock :
       linux_port::PresentationCommandPacket::PhasePolicy::Static,
       o.cameraParallaxX * (o.flipX ? -1.f : 1.f),o.cameraParallaxY));
   },
   [&](const game::ParticleSpray& spray,const game::ParticleSpray::Particle& p,const render::Sprite&) {
     auto key=linux_port::EmittedParticleKey(spray,p);
     key.object |= (std::uint64_t{1}<<62);
     assert(recorder.TagLastSprite(key));
   });
 recorder.EndFrame();
 return {recorder.Packet(),scene.SceneEpoch(),cameraX*(2.f/3.f),-cameraY*(2.f/3.f),false};
}
int main() {
 // Isolated complete packet: immutable copies and identical-alpha calls admit.
 auto inner=std::make_unique<Spy>(); linux_port::PresentationRecordingRenderer r(std::move(inner));
 render::Sprite s; s.texture=1; s.destination={10,20,30,40};
 r.BeginFrame({}); r.DrawSprite(s); assert(r.TagLastSprite({7,1})); r.EndFrame();
 linux_port::FullFrameSnapshot a{r.Packet(),1,0,0,false};
 r.BeginFrame({}); s.destination.x=20; r.DrawSprite(s); assert(r.TagLastSprite({7,1})); r.EndFrame();
 linux_port::FullFrameSnapshot b{r.Packet(),1,0,0,false};
 assert(r.HasPreviousPacket() && r.PreviousPacket().Commands()[1].sprite.destination.x==10);
 assert(linux_port::SampleFullFrame(a,b,.5f).Commands()[1].sprite.destination.x==15);
 assert(linux_port::SampleFullFrame(a,b,1.f).Commands()[1].sprite.destination.x==20);
 assert(linux_port::AdmitFullFrame(a,b,.5f)==linux_port::FullFrameDecision::Interpolate);
 assert(linux_port::AdmitFullFrame(a,b,.5f)==linux_port::AdmitFullFrame(a,b,.5f));
 assert(a.packet.Commands()[1].sprite.destination.x==10);
 auto birth=b; birth.packet.Clear(); birth.packet.Add(linux_port::PresentationCommandPacket::Command{linux_port::PresentationCommandPacket::Kind::Begin});
 birth.packet.Seal(); assert(linux_port::AdmitFullFrame(a,birth,.5f)==linux_port::FullFrameDecision::HoldCurrent);
 auto painter=b; painter.packet.Clear();
 painter.packet.Add(linux_port::PresentationCommandPacket::Command{linux_port::PresentationCommandPacket::Kind::Begin});
 auto changed=b.packet.Commands()[1]; changed.key={8,1}; painter.packet.Add(changed); painter.packet.Seal();
 assert(linux_port::AdmitFullFrame(a,painter,.5f)==linux_port::FullFrameDecision::HoldCurrent);
 auto generation=b; generation.packet.Clear();
 generation.packet.Add(linux_port::PresentationCommandPacket::Command{linux_port::PresentationCommandPacket::Kind::Begin});
 changed=b.packet.Commands()[1]; changed.key={7,2}; generation.packet.Add(changed); generation.packet.Seal();
 assert(linux_port::AdmitFullFrame(a,generation,.5f)==linux_port::FullFrameDecision::HoldCurrent);
 auto cut=b; cut.discontinuity=true; assert(linux_port::AdmitFullFrame(a,cut,.5f)==linux_port::FullFrameDecision::HoldCurrent);
 auto epoch=b; epoch.sceneEpoch=2; assert(linux_port::AdmitFullFrame(a,epoch,.5f)==linux_port::FullFrameDecision::HoldCurrent);
 auto untagged=b; untagged.packet.Clear(); untagged.packet.Add(linux_port::PresentationCommandPacket::Command{linux_port::PresentationCommandPacket::Kind::Begin});
 linux_port::PresentationCommandPacket::Command raw{linux_port::PresentationCommandPacket::Kind::Sprite}; raw.sprite=s;
 untagged.packet.Add(raw); untagged.packet.Seal();
 assert(linux_port::AdmitFullFrame(a,untagged,.5f)==linux_port::FullFrameDecision::HoldCurrent);
 // Repeat sprite and shader-clock phases use the same alpha without update.
 r.BeginFrame({}); s.repeatX=true; s.uvShiftX=0.f; s.foliageSway.enabled=true;
 s.foliageSway.time=1.f; s.pixelWobble.time=2.f;
 r.DrawSprite(s); assert(r.TagLastSprite({44,0},1.f,1.f,
     linux_port::PresentationCommandPacket::PhasePolicy::ShaderClock)); r.EndFrame();
 linux_port::FullFrameSnapshot shaderA{r.Packet(),3,0,0,false,1.f,2.f};
 r.BeginFrame({}); s.destination.x=24; s.foliageSway.time=1.016f; s.pixelWobble.time=2.016f;
 r.DrawSprite(s); assert(r.TagLastSprite({44,0},1.f,1.f,
     linux_port::PresentationCommandPacket::PhasePolicy::ShaderClock)); r.EndFrame();
 linux_port::FullFrameSnapshot shaderB{r.Packet(),3,0,0,false,1.016f,2.016f};
 assert(linux_port::AdmitFullFrame(shaderA,shaderB,.5f)==linux_port::FullFrameDecision::Interpolate);
 auto shaderMid=linux_port::SampleFullFrame(shaderA,shaderB,.5f);
 assert(shaderMid.Commands()[1].sprite.destination.x==22.f);
 assert(std::fabs(shaderMid.Commands()[1].sprite.foliageSway.time-1.008f)<.00001f);
 auto badClock=shaderB; badClock.shaderTime=9.f;
 assert(linux_port::AdmitFullFrame(shaderA,badClock,.5f)==linux_port::FullFrameDecision::HoldCurrent);
 setenv("DEATHLEAK_RENDER_ONLY_XML_IDS","1,2,17,19,37,47",1);
 auto spy=std::make_unique<Spy>(); linux_port::PresentationRecordingRenderer recorder(std::move(spy));
 game::LevelScene scene; std::string error;
 assert(scene.Load("Game/XmlFiles/levels/forest_exploration_1.xml",recorder,error));
 auto first=Capture(scene,recorder,scene.CameraX(),scene.CameraY());
 auto second=Capture(scene,recorder,scene.CameraX()+10.f,scene.CameraY());
 std::size_t sprites=0,tagged=0;
 for (const auto& c:second.packet.Commands()) if (c.kind==linux_port::PresentationCommandPacket::Kind::Sprite) {
   ++sprites; tagged += c.key.object!=0;
 }
 assert(sprites>0 && tagged>0);
 const auto result=linux_port::AdmitFullFrame(first,second,.5f);
 for (std::size_t i=0;i<first.packet.Size() && i<second.packet.Size();++i) {
   const auto& x=first.packet.Commands()[i]; const auto& y=second.packet.Commands()[i];
   if(x.kind==linux_port::PresentationCommandPacket::Kind::Sprite &&
      !linux_port::SameSpriteExceptPosition(x.sprite,y.sprite))
      std::cout<<"nonpositional sprite change at painter command "<<i<<" key="<<x.key.object
               <<" uv="<<x.sprite.uvShiftX<<"->"<<y.sprite.uvShiftX<<"\n";
 }
 // The emitted Forest parallax phases are camera-derived; admit this bounded
 // scene packet, while complete game packets with unowned HUD/combat still hold.
 assert(result==linux_port::FullFrameDecision::Interpolate);
 auto sampled=linux_port::SampleFullFrame(first,second,.5f);
 assert(sampled.Sealed() && sampled.Size()==second.packet.Size());
 auto phaseBad=second;
 phaseBad.packet.Clear();
 for (const auto& c:second.packet.Commands()) {
   auto changed=c;
   if (changed.kind==linux_port::PresentationCommandPacket::Kind::Sprite &&
       changed.phase==linux_port::PresentationCommandPacket::PhasePolicy::CameraParallax)
      changed.sprite.uvShiftX += .01f;
   phaseBad.packet.Add(changed);
 }
 phaseBad.packet.Seal();
 assert(linux_port::AdmitFullFrame(first,phaseBad,.5f)==linux_port::FullFrameDecision::HoldCurrent);
 auto full=second;
 full.packet.Clear();
 for(const auto& c:second.packet.Commands()) full.packet.Add(c);
 linux_port::PresentationCommandPacket::Command hud{linux_port::PresentationCommandPacket::Kind::Sprite};
 hud.sprite.texture=17; full.packet.Add(hud); full.packet.Seal();
 assert(linux_port::AdmitFullFrame(first,full,.5f)==linux_port::FullFrameDecision::HoldCurrent);
 using Clock=linux_port::FixedStepPacer::Clock;
 for(int hz:{30,60,75,120,144,165,240}) {
   linux_port::FixedStepPacer p(Clock::time_point{}); int updates=0;
   for(int slot=0;slot<=hz*10;++slot) {
     p.Accumulate(Clock::time_point{}+std::chrono::duration_cast<Clock::duration>(std::chrono::duration<double>(double(slot)/hz)));
     while(p.Due()) { p.Consume(); ++updates; }
   }
   assert(updates==601 && p.DroppedSteps()==0);
 }
 std::cout<<"full-frame real Forest packet: "<<tagged<<"/"<<sprites<<" sprites tagged; moving-camera scene admission=interpolate; full-game unowned=hold; 30/60/75/120/144/165/240 cadence passed\n";
}
