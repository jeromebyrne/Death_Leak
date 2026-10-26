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
 auto a=frame(0,393,223), b=frame(10,394,224,true);
 auto d=AdmitMixedCommands(a,b,.5f); assert(d.valid && d.moving==1 && d.interpolate[1]);
 auto sample=SampleMixedCommands(a,b,.5f); assert(sample.Sealed() && sample.Size()==b.packet.Size());
 assert(sample.Commands()[1].sprite.destination.x==5);
 assert(sample.Commands()[2].sprite.texture==224 && sample.Commands()[3].sprite.texture==394);
 assert(sample.Commands()[4].key.object==4);
 auto exact=SampleMixedCommands(a,b,1.f); assert(exact.Commands()[1].sprite.destination.x==10 && exact.Commands()[4].key.object==4);
 auto cut=b; cut.discontinuity=true; assert(AdmitMixedCommands(a,cut,.5f).moving==0);
 auto epoch=b; epoch.sceneEpoch=2; assert(AdmitMixedCommands(a,epoch,.5f).moving==0);
 auto invalid=AdmitMixedCommands(a,b,-1.f); assert(!invalid.valid);
 auto reorder=b; reorder.packet.Clear(); for(auto c:b.packet.Commands()) { if(c.key.object==1) c.key={2,1}; else if(c.key.object==2) c.key={1,1}; reorder.packet.Add(c); } reorder.packet.Seal(); assert(AdmitMixedCommands(a,reorder,.5f).moving==0);
 auto blend=b; blend.packet.Clear(); for(auto c:b.packet.Commands()) { if(c.key.object==1) c.sprite.opacity=.5f; blend.packet.Add(c); } blend.packet.Seal(); assert(AdmitMixedCommands(a,blend,.5f).moving==0);
 auto death=a; death.packet.Clear(); for(auto c:a.packet.Commands()) { if(c.key.object!=2) death.packet.Add(c); } death.packet.Seal(); assert(AdmitMixedCommands(a,death,.5f).moving==1);
 auto unowned=b; unowned.packet.Clear(); for(auto c:b.packet.Commands()) { if(c.kind==Kind::Sprite && c.key.object==4) c.key={}; unowned.packet.Add(c); } unowned.packet.Seal();
 assert(AdmitMixedCommands(a,unowned,.5f).moving==0);
 assert(SampleMixedCommands(a,unowned,.5f).Commands()[1].sprite.destination.x==10);
 auto previouslyUnowned=a; previouslyUnowned.packet.Clear(); for(auto c:a.packet.Commands()) { if(c.kind==Kind::Sprite && c.key.object==2) c.key={}; previouslyUnowned.packet.Add(c); } previouslyUnowned.packet.Seal();
 assert(AdmitMixedCommands(previouslyUnowned,b,.5f).moving==0);
 auto batch=b; batch.packet.Clear(); for(auto c:b.packet.Commands()) batch.packet.Add(c);
 PresentationCommandPacket::Command unownedBatch{Kind::SpriteBatch}; unownedBatch.sprites.push_back(b.packet.Commands()[1].sprite); batch.packet.Add(unownedBatch); batch.packet.Seal();
 assert(AdmitMixedCommands(a,batch,.5f).moving==0);
 std::cout<<"mixed ordered packet: stable sampled; film/animation/birth held; cuts held; alpha=1 current\n";
}
