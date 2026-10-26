#include "../linux/MixedCommandInterpolationGate.h"
#include <cassert>
#include <iostream>
using namespace deathleak::linux_port;
using Kind=PresentationCommandPacket::Kind;
static FullFrameSnapshot frame(float x,int film,int actor, bool extra=false) {
 FullFrameSnapshot f; f.sceneEpoch=1;
 PresentationCommandPacket::Command begin{Kind::Begin}; f.packet.Add(begin);
 PresentationCommandPacket::Command stable{Kind::Sprite}; stable.key={1,1}; stable.sprite.texture=1;
 stable.sprite.destination={x,20,30,40}; f.packet.Add(stable);
 PresentationCommandPacket::Command anim{Kind::Sprite}; anim.key={2,1}; anim.sprite.texture=actor;
 anim.sprite.destination={100,0,20,20}; f.packet.Add(anim);
 PresentationCommandPacket::Command filmCmd{Kind::Sprite}; filmCmd.key={3,1}; filmCmd.sprite.texture=film;
 filmCmd.sprite.destination={0,0,1280,720}; f.packet.Add(filmCmd);
 if(extra) { PresentationCommandPacket::Command born{Kind::Sprite}; born.key={4,1}; born.sprite.texture=4; f.packet.Add(born); }
 f.packet.Seal(); return f;
}
int main() {
 auto a=frame(0,393,223), b=frame(10,393,223,true);
 auto d=AdmitMixedCommands(a,b,.5f); assert(d.valid && d.moving==3 && d.interpolate[1]);
 auto sample=SampleMixedCommands(a,b,.5f); assert(sample.Sealed() && sample.Size()==b.packet.Size());
 assert(sample.Commands()[1].sprite.destination.x==5);
 assert(sample.Commands()[2].sprite.texture==223 && sample.Commands()[3].sprite.texture==393);
 assert(sample.Commands()[4].key.object==4);
 auto exact=SampleMixedCommands(a,b,1.f); assert(exact.Commands()[1].sprite.destination.x==10 && exact.Commands()[4].key.object==4);
 auto actorFrame=b; actorFrame.packet.Clear(); for(auto c:b.packet.Commands()) {
  if(c.key.object==2) c.sprite.texture=224;
  actorFrame.packet.Add(c); } actorFrame.packet.Seal();
 assert(AdmitMixedCommands(a,actorFrame,.5f).moving==0);
 assert(SampleMixedCommands(a,actorFrame,.5f).Commands()[1].sprite.destination.x==10);
 auto filmFrame=b; filmFrame.packet.Clear(); for(auto c:b.packet.Commands()) {
  if(c.key.object==3) c.sprite.texture=394;
  filmFrame.packet.Add(c); } filmFrame.packet.Seal();
 assert(AdmitMixedCommands(a,filmFrame,.5f).moving==0);
 auto cut=b; cut.discontinuity=true; assert(AdmitMixedCommands(a,cut,.5f).moving==0);
 auto epoch=b; epoch.sceneEpoch=2; assert(AdmitMixedCommands(a,epoch,.5f).moving==0);
 auto invalid=AdmitMixedCommands(a,b,-1.f); assert(!invalid.valid);
 auto reorder=b; reorder.packet.Clear(); for(auto c:b.packet.Commands()) { if(c.key.object==1) c.key={2,1}; else if(c.key.object==2) c.key={1,1}; reorder.packet.Add(c); } reorder.packet.Seal(); assert(AdmitMixedCommands(a,reorder,.5f).moving==0);
 auto blend=b; blend.packet.Clear(); for(auto c:b.packet.Commands()) { if(c.key.object==1) c.sprite.opacity=.5f; blend.packet.Add(c); } blend.packet.Seal(); assert(AdmitMixedCommands(a,blend,.5f).moving==0);
 auto death=a; death.packet.Clear(); for(auto c:a.packet.Commands()) { if(c.key.object!=2) death.packet.Add(c); } death.packet.Seal(); assert(AdmitMixedCommands(a,death,.5f).moving==0);
 auto unowned=b; unowned.packet.Clear(); for(auto c:b.packet.Commands()) { if(c.kind==Kind::Sprite && c.key.object==4) c.key={}; unowned.packet.Add(c); } unowned.packet.Seal();
 assert(AdmitMixedCommands(a,unowned,.5f).moving==0);
 assert(SampleMixedCommands(a,unowned,.5f).Commands()[1].sprite.destination.x==10);
 auto previouslyUnowned=a; previouslyUnowned.packet.Clear(); for(auto c:a.packet.Commands()) { if(c.kind==Kind::Sprite && c.key.object==2) c.key={}; previouslyUnowned.packet.Add(c); } previouslyUnowned.packet.Seal();
 assert(AdmitMixedCommands(previouslyUnowned,b,.5f).moving==0);
 auto batch=b; batch.packet.Clear(); for(auto c:b.packet.Commands()) batch.packet.Add(c);
 PresentationCommandPacket::Command unownedBatch{Kind::SpriteBatch}; unownedBatch.sprites.push_back(b.packet.Commands()[1].sprite); batch.packet.Add(unownedBatch); batch.packet.Seal();
 assert(AdmitMixedCommands(a,batch,.5f).moving==0);
 // Authored projectile fade-in and heading are sampled only for one stable birth.
 auto projectileA=frame(0,393,223), projectileB=frame(0,393,223);
 auto retag=[](FullFrameSnapshot& f, float px,float width,float degrees,int alpha) {
  auto commands=f.packet.Commands(); f.packet.Clear();
  for(auto c:commands) { if(c.key.object==1) { c.key={(std::uint64_t{1}<<59)|7,0};
   c.sprite.destination.x=px; c.sprite.destination.width=width;
   c.sprite.destination.height=width/2; c.sprite.rotationDegrees=degrees;
   c.sprite.tint.a=static_cast<std::uint8_t>(alpha); } f.packet.Add(c); }
  f.packet.Seal();
 };
 retag(projectileA,0,40,350,120); retag(projectileB,20,44,10,132);
 auto pd=AdmitMixedCommands(projectileA,projectileB,.5f); assert(pd.moving>=1 && pd.interpolate[1]);
 auto ps=SampleMixedCommands(projectileA,projectileB,.5f);
 assert(ps.Commands()[1].sprite.destination.x==10 && ps.Commands()[1].sprite.destination.width==42);
 assert(ps.Commands()[1].sprite.rotationDegrees==360 && ps.Commands()[1].sprite.tint.a==126);
 auto pExact=SampleMixedCommands(projectileA,projectileB,1.f);
 assert(pExact.Commands()[1].sprite.rotationDegrees==10 && pExact.Commands()[1].sprite.tint.a==132);
 auto flipped=projectileB; flipped.packet.Clear(); for(auto c:projectileB.packet.Commands()) { if(c.key.object>>59==1) c.sprite.flipX=true; flipped.packet.Add(c); } flipped.packet.Seal();
 assert(!AdmitMixedCommands(projectileA,flipped,.5f).interpolate[1]);
 auto cutProjectile=projectileB; cutProjectile.sceneEpoch=2;
 assert(AdmitMixedCommands(projectileA,cutProjectile,.5f).moving==0);
 auto bornEffect=projectileB; bornEffect.packet.Clear();
 for(auto c:projectileB.packet.Commands()) bornEffect.packet.Add(c);
 PresentationCommandPacket::Command effect{Kind::Sprite}; effect.key={12345,1}; bornEffect.packet.Add(effect); bornEffect.packet.Seal();
 assert(!AdmitMixedCommands(projectileA,bornEffect,.5f).interpolate[1]);
 // Stable particle lifetime: interpolate square size and fade, but not a
 // recycled generation, painter topology, flip or any discrete GPU state.
 auto particleRetag=[](FullFrameSnapshot& f, std::uint64_t ns, float px,float size,int alpha) {
  auto commands=f.packet.Commands(); f.packet.Clear();
  for(auto c:commands) { if(c.key.object==1) { c.key={ns|7,3};
   c.sprite.destination={px,20,size,size}; c.sprite.tint.a=static_cast<std::uint8_t>(alpha); }
   f.packet.Add(c); } f.packet.Seal();
 };
 for(const auto ns:{std::uint64_t{1}<<60,std::uint64_t{1}<<62}) {
  auto particleA=frame(0,393,223), particleB=frame(0,393,223);
  particleRetag(particleA,ns,0,30,180); particleRetag(particleB,ns,12,26,164);
  assert(AdmitMixedCommands(particleA,particleB,.5f).interpolate[1]);
  const auto half=SampleMixedCommands(particleA,particleB,.5f);
  assert(half.Commands()[1].sprite.destination.x==6 &&
         half.Commands()[1].sprite.destination.width==28 &&
         half.Commands()[1].sprite.tint.a==172);
  assert(SampleMixedCommands(particleA,particleB,1.f).Commands()[1].sprite.tint.a==164);
  auto recycled=particleB; recycled.packet.Clear(); for(auto c:particleB.packet.Commands()) {
   if(c.key.object==(ns|7)) ++c.key.generation;
   recycled.packet.Add(c); } recycled.packet.Seal();
  assert(!AdmitMixedCommands(particleA,recycled,.5f).interpolate[1]);
  auto state=particleB; state.packet.Clear(); for(auto c:particleB.packet.Commands()) {
   if(c.key.object==(ns|7)) c.sprite.flipX=!c.sprite.flipX;
   state.packet.Add(c); } state.packet.Seal();
  assert(!AdmitMixedCommands(particleA,state,.5f).interpolate[1]);
  auto jump=particleB; jump.packet.Clear(); for(auto c:particleB.packet.Commands()) {
   if(c.key.object==(ns|7)) c.sprite.tint.a=20;
   jump.packet.Add(c); } jump.packet.Seal();
  assert(!AdmitMixedCommands(particleA,jump,.5f).interpolate[1]);
 }
 // A source-stamped fade/aim may sample alpha, opacity and shortest rotation;
 // an untagged tint/rotation change remains discrete current state.
 auto phaseA=frame(0,393,223), phaseB=frame(10,393,223);
 auto stamp=[](FullFrameSnapshot& f,int tint,float opacity,float angle,bool allow) {
  auto commands=f.packet.Commands(); f.packet.Clear(); for(auto c:commands) {
   if(c.key.object==1) { c.continuousAlpha=allow; c.continuousRotation=allow;
    c.sprite.tint.a=static_cast<std::uint8_t>(tint); c.sprite.opacity=opacity;
    c.sprite.rotationDegrees=angle; } f.packet.Add(c); } f.packet.Seal(); };
 stamp(phaseA,180,.7f,350,true); stamp(phaseB,170,.68f,10,true);
 assert(AdmitMixedCommands(phaseA,phaseB,.5f).interpolate[1]);
 auto phaseHalf=SampleMixedCommands(phaseA,phaseB,.5f);
 assert(phaseHalf.Commands()[1].sprite.tint.a==175 &&
        phaseHalf.Commands()[1].sprite.rotationDegrees==360);
 stamp(phaseB,170,.68f,10,false);
 assert(!AdmitMixedCommands(phaseA,phaseB,.5f).interpolate[1]);
 std::cout<<"mixed ordered packet: stable sampled; film/animation/birth held; cuts held; alpha=1 current\n";
}
