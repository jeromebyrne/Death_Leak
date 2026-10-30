// CPU-only, source-authored campaign fixtures. No display, saves or GPU.
#include "../game/LevelScene.h"
#include "../../Game/tinyxml/tinyxml.h"
#include <cassert>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <limits>
#include <unordered_map>
namespace g=deathleak::game;
namespace r=deathleak::render;
struct Spy final : r::Renderer {
 std::unordered_map<std::string,r::TextureId> textures;
 std::vector<r::Sprite> draws;
 bool Initialize(const char*,int,int,bool,std::string&) override{return true;}
 r::TextureId LoadTexture(const std::string& path,std::string&) override {
  assert(std::filesystem::exists(path));
  auto [it,inserted]=textures.emplace(path,static_cast<r::TextureId>(textures.size()+1));return it->second;
 }
 void BeginFrame(r::Color) override{} void DrawSprite(const r::Sprite& s) override{draws.push_back(s);}
 void FillRect(r::Rect,r::Color) override{} void ApplyScenePostprocess() override{}
 void EndFrame() override{} bool PollQuit() override{return false;}
 bool KeyDown(int) const override{return false;} r::MouseState Mouse() const override{return {};}
 bool SaveFrame(const std::string&,std::string&) override{return false;}
};
bool near(float a,float b){return std::abs(a-b)<.0001f;}
size_t indexOf(const g::LevelScene& s,const char* id) {
 for(size_t i=0;i<s.SceneSprites().size();++i)if(s.SceneSprites()[i].xmlId==id)return i;
 assert(false);return 0;
}
g::LevelScene::AmbientTarget target(const g::LevelScene& s,size_t index) {
 for(const auto& t:s.AmbientTargets())if(t.index==index)return t;
 assert(false);return {};
}
int main() {
 Spy spy;g::LevelScene scene;std::string error;
 assert(scene.Load("Game/XmlFiles/levels/hot_springs.xml",spy,error));
 assert(scene.AmbientTargets().size()==2);
 const size_t first=indexOf(scene,"32"),second=indexOf(scene,"33");
 const auto snapshot=scene.AmbientTargets();const auto a=target(scene,first),b=target(scene,second);
 assert(a.butterfly && b.butterfly && near(a.x,47.451904f) && near(a.y,29.f));
 assert(near(a.width,26.509336f) && near(a.height,26.509336f));
 assert(a.offsetX==0 && a.offsetY==0);
 const auto* originalStorage=scene.SceneSprites().data();const auto count=scene.SceneSprites().size();
 const size_t orbs=scene.PendingSpawnedOrbs();
 // Source Butterfly::OnDamage ignores health: even zero damage retires.
 assert(scene.DamageAmbient(first,0,spy));
 assert(scene.AmbientTargets().size()==1 && scene.AmbientTargets()[0].index==second);
 assert(snapshot.size()==2); // by-value snapshot cannot dangle after retirement.
 assert(!scene.DamageAmbient(first,1,spy));
 assert(!scene.DamageAmbient(count,1,spy));
 assert(!scene.DamageAmbient(second,-1,spy));
 assert(!scene.DamageAmbient(second,std::numeric_limits<float>::quiet_NaN(),spy));
 assert(scene.SceneSprites().data()==originalStorage && scene.SceneSprites().size()==count);
 assert(scene.SceneSprites()[first].ambientRetired && !scene.SceneSprites()[first].collected);
 assert(scene.PendingSpawnedOrbs()==orbs);
 assert(scene.AmbientDebrisBodies().size()==2);
 for(size_t i=0;i<2;++i) {
  const auto& d=scene.AmbientDebrisBodies()[i];
  assert(d.texture==spy.textures.at("Game/Media/characters/butterfly/dead_wing_"+std::to_string(i+1)+".png"));
  assert(d.width==32 && d.height==32 && d.collisionWidth==15 && d.collisionHeight==15);
  assert(d.depth==4500 && near(d.x,a.x+i*10.f) && near(d.y,a.y));
  assert(std::abs(d.vx)>=.75f && std::abs(d.vx)<=3.6f && near(d.vy,std::abs(d.vx)*.5f));
 }
 bool drewDead=false,drewLive=false;spy.draws.clear();
 scene.Draw(spy,0,0,0,0,{},1,1,{},false,{},[&](const auto& s,const auto&) {
  if(s.xmlId=="32")drewDead=true;
  if(s.xmlId=="33")drewLive=true;
 });
 assert(!drewDead && drewLive);
 for(const auto& d:scene.AmbientDebrisBodies()) {
  bool rendered=false;for(const auto& draw:spy.draws)if(draw.texture==d.texture) {
   rendered=true;assert(near(draw.destination.width,32.f*2.f/3.f) && draw.flipY);
  }assert(rendered);
 }
 const auto deadX=scene.SceneSprites()[first].x,deadY=scene.SceneSprites()[first].y;
 const auto debrisBefore=scene.AmbientDebrisBodies().front();
 scene.UpdateDynamics(1.f/60.f,0,0,0,0);
 const auto& debrisAfter=scene.AmbientDebrisBodies().front();
 assert(near(debrisAfter.x,debrisBefore.x+debrisBefore.vx));
 assert(near(debrisAfter.y,debrisBefore.y+debrisBefore.vy));
 assert(near(debrisAfter.vx,debrisBefore.vx*.95f));
 assert(near(debrisAfter.ay,-1.f)); // deferred source gravity, tick units.
 assert(scene.SceneSprites()[first].x==deadX && scene.SceneSprites()[first].y==deadY);
 bool touched=false,shrunk=false;
 for(int i=0;i<1200 && !scene.AmbientDebrisBodies().empty();++i) {
  for(const auto& d:scene.AmbientDebrisBodies()) {
   if(d.fading)touched=true;
   if(d.fading && d.fadeRemaining<.2f) {
    shrunk=true;spy.draws.clear();scene.Draw(spy,0,0,0,0);
    bool found=false;for(const auto& draw:spy.draws)if(draw.texture==d.texture) {
     found=true;assert(draw.destination.width<32.f*2.f/3.f);
    }assert(found);
   }
  }
  scene.UpdateDynamics(1.f/60.f,0,0,0,0);
 }
 assert(touched && shrunk && scene.AmbientDebrisBodies().empty());
 // The campaign hot_springs file has NO background butterflies: both are
 // kGround. Preserve that evidence and derive depth-only immunity fixtures.
 // Exact != kGround also rejects FOREGROUND, not only numeric background.
 for(const char* depth:{"kGroundBack","kFarBackground","kNearForeground"}) {
  TiXmlDocument doc("Game/XmlFiles/levels/hot_springs.xml");assert(doc.LoadFile());
  auto* root=doc.RootElement();assert(root);
  auto* butterfly=root->FirstChildElement("Butterfly");assert(butterfly);
  assert(std::string(butterfly->Attribute("id"))=="32");
  butterfly->FirstChildElement("position")->SetAttribute("depth_layer",depth);
  std::filesystem::create_directories("port/build/ambient-damage-fixtures");
  const std::string path=std::string("port/build/ambient-damage-fixtures/ambient_depth_")+depth+".xml";
  assert(doc.SaveFile(path.c_str()));assert(scene.Load(path,spy,error));
  assert(scene.AmbientDebrisBodies().empty());
  const auto immune=indexOf(scene,"32");assert(scene.AmbientTargets().size()==1);
  assert(!scene.DamageAmbient(immune,100,spy));assert(!scene.SceneSprites()[immune].ambientRetired);
 }
 // Rat artwork is authored on the Rabbit type; do not replace it with rabbit
 // or collectible art. Original health=.01 and offsetX=5 survive the bridge.
 assert(scene.Load("Game/XmlFiles/levels/catacombs_entrance_1.xml",spy,error));
 assert(scene.AmbientTargets().size()==2 && scene.AmbientDebrisBodies().empty());
 const auto rat=indexOf(scene,"25"),other=indexOf(scene,"26");
 const auto ratTarget=target(scene,rat);assert(!ratTarget.butterfly);
 assert(near(ratTarget.width,36.741764f) && near(ratTarget.height,13.270956f) && ratTarget.offsetX==5);
 const auto* ratStorage=scene.SceneSprites().data();const auto ratSize=scene.SceneSprites().size();
 assert(scene.DamageAmbient(rat,.005f,spy));assert(scene.AmbientTargets().size()==2);
 assert(scene.AmbientDebrisBodies().empty());
 assert(scene.DamageAmbient(rat,.005f,spy));assert(scene.AmbientTargets().size()==1);
 assert(scene.AmbientTargets().front().index==other && scene.AmbientDebrisBodies().size()==10);
 assert(scene.PendingSpawnedOrbs()==0);
 for(const auto& d:scene.AmbientDebrisBodies()) {
  assert(d.width==20 || d.width==25 || d.width==30);
  const int variant=(int(d.width)-20)/5;
  const std::string art="Game/Media/orb"+(variant==0 ? std::string{} : std::to_string(variant+1))+".png";
  assert(d.texture==spy.textures.at(art) && d.depth==4500);
  assert(d.collisionWidth==(variant==0 ? 15.f : d.width));
 }
 assert(scene.SceneSprites().data()==ratStorage && scene.SceneSprites().size()==ratSize);
 assert(!scene.DamageAmbient(indexOf(scene,"2"),1,spy)); // Player is not ambient.
 assert(scene.Load("Game/XmlFiles/levels/inside_hanging_shadow.xml",spy,error));
 assert(scene.AmbientTargets().size()==2 && scene.AmbientDebrisBodies().empty());
 std::cout<<"ambient_damage_bridge_test: PASS (campaign targets, retirement, native wings, debris motion/fade, depth immunity, stable identity, no rewards)\n";
}
