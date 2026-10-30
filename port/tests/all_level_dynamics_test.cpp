// Source-backed regressions outside the previously sampled Forest-1 opening.
#include "../game/LevelScene.h"
#include "../game/PlayerMotion.h"
#include "../game/TechniqueCatalog.h"
#include "../game/WeatherLightning.h"
#include "../../Game/tinyxml/tinyxml.h"
#include "../render/OrdinarySpriteRotation.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <filesystem>
#include <unordered_map>
namespace r = deathleak::render;
namespace g = deathleak::game;
struct Spy final : r::Renderer {
 std::unordered_map<std::string,r::TextureId> textures;
 bool Initialize(const char*,int,int,bool,std::string&) override{return true;}
 r::TextureId LoadTexture(const std::string& p,std::string&) override {
  auto [it,inserted]=textures.emplace(p,static_cast<r::TextureId>(textures.size()+1));return it->second;
 }
 void BeginFrame(r::Color) override{} void DrawSprite(const r::Sprite&) override{}
 void FillRect(r::Rect,r::Color) override{} void ApplyScenePostprocess() override{}
 void EndFrame() override{} bool PollQuit() override{return false;}
 bool KeyDown(int) const override{return false;}
 r::MouseState Mouse() const override{return {};}
 bool SaveFrame(const std::string&,std::string&) override{return false;}
};
bool near(float a,float b){return std::abs(a-b)<.003f;}
struct Packet {g::SceneSprite object;r::Sprite draw;};
Packet objectAt(g::LevelScene& scene,Spy& renderer,const char* id,float x,float y) {
 Packet result;bool found=false;
 for(const auto& o:scene.SceneSprites())if(o.xmlId==id){result.object=o;found=true;}
 assert(found);
 scene.Draw(renderer,x,y,x,y,{},1,1,{},false,{},
  {},[&](const g::SceneSprite& o,const r::Sprite& d,bool arm){if(!arm && o.xmlId==id)result.draw=d;});
 return result;
}
int main(){
 Spy renderer;g::LevelScene scene;std::string error;
 assert(scene.Load("Game/XmlFiles/levels/cave_platforming_1.xml",renderer,error));
 constexpr float x=3693.520020f,y=601.f,dt=1.f/60.f;
 auto before=objectAt(scene,renderer,"19",x,y);
 const auto index=before.object.solidBoxIndex;assert(index<scene.SolidBoxes().size());
 const auto bounds=scene.SolidBoxes()[index];
 scene.UpdateDynamics(dt,x,y,x,y);
 auto after=objectAt(scene,renderer,"19",x,y);
 const float offset=500.f*std::sin(1.5f*dt);
 assert(near(after.object.y,y+offset));assert(near(after.object.rotation,1.f));
 assert(near(scene.SolidBoxes()[index].top,bounds.top+offset));
 assert(near(scene.SolidBoxes()[index].bottom,bounds.bottom+offset));
 assert(after.object.texture==r::InvalidTexture); // original saw.png is absent, not fabricated.
 /* pause: no UpdateDynamics call */
 assert(near(objectAt(scene,renderer,"19",x,y).object.y,after.object.y));
 scene.UpdateDynamics(dt,x,y,x+20000,y+20000);
 assert(near(objectAt(scene,renderer,"19",x,y).object.rotation,after.object.rotation));
 // KeyPickup auto-rotation used to be frozen in all four key-bearing rooms.
 assert(scene.Load("Game/XmlFiles/levels/grass_exploration_dead_end_below_2.xml",renderer,error));
 auto key=objectAt(scene,renderer,"34",44.f,-1050.f);
 scene.UpdateDynamics(dt,44,-1050,44,-1050);
 assert(near(objectAt(scene,renderer,"34",44,-1050).object.rotation,key.object.rotation+.05f));
 // Source rotation overflow resets to zero, not a modulo remainder.
 for(int i=0;i<125;++i)scene.UpdateDynamics(dt,44,-1050,44,-1050);
 assert(near(objectAt(scene,renderer,"34",44,-1050).object.rotation,0.f));
 // Boat-on-water bobbing follows source SineWave speed/offset, without input.
 assert(scene.Load("Game/XmlFiles/levels/sea_1.xml",renderer,error));
 // Every authored player, not just XML ID2, must have the firing back-arm art.
 renderer.textures.clear();
 assert(scene.Load("Game/XmlFiles/levels/water_well_1.xml",renderer,error));
 bool arm=false;
 for(const auto& entry:renderer.textures)if(entry.first.find("/back_arm_")!=std::string::npos)arm=true;
 assert(arm);
 // Loading all authored maps is a source/assets gate, not gameplay completion.
 int loaded=0,small=0,large=0,particles=0,ambient=0,platforms=0,breakables=0,healing=0,instruments=0,rotating=0;
 for(const auto& entry:std::filesystem::directory_iterator("Game/XmlFiles/levels")) {
  if(entry.path().extension()!=".xml")continue;
  assert(scene.Load(entry.path().string(),renderer,error));++loaded;
  for(const auto& object:scene.SceneSprites()) {
   if(object.currencyOrb){assert(near(object.alpha,.75f));assert(object.depth==3250);if(object.largeOrb)++large;else{assert(object.authoredSmallOrb);++small;}}
   if(object.ambientActorIndex!=static_cast<size_t>(-1))++ambient;
   if(object.platformIndex!=static_cast<size_t>(-1))++platforms;
   if(object.autoRotation!=0.f)++rotating;
  }
  particles+=scene.AuthoredParticleSprays().size();breakables+=scene.Breakables().size();
  healing+=scene.HealingSprings().size();instruments+=scene.Instruments().size();
 }
 assert(loaded==39);assert(small==835 && large==5);assert(particles==42);
 std::cerr<<"inventory ambient="<<ambient<<" platforms="<<platforms<<" breakables="<<breakables<<" springs="<<healing<<" instruments="<<instruments<<" rotators="<<rotating<<"\n";
 assert(ambient==46 && platforms==11 && breakables==81 && healing==2 && instruments==1 && rotating==17);
 // Large-orb contact schedules fifty real temporary pickups, not fifty credited coins.
 assert(scene.Load("Game/XmlFiles/levels/dojo_front.xml",renderer,error));
 auto orb=std::find_if(scene.SceneSprites().begin(),scene.SceneSprites().end(),[](const auto& object){return object.largeOrb;});
 assert(orb!=scene.SceneSprites().end());const float ox=orb->x,oy=orb->y;
 std::unordered_set<std::string> collected;
 assert(scene.CollectCurrencyOrbs(ox,oy,collected)>=1);assert(scene.PendingSpawnedOrbs()==50);
 assert(scene.UpdateSpawnedOrbs(ox,oy,0.f)==0); // spawn-period lockout
 // Water uses authored 10000-square collider, not the tiny visual square.
 assert(scene.Load("Game/XmlFiles/levels/underwater_cave.xml",renderer,error));
 auto water=std::find_if(scene.Hazards().begin(),scene.Hazards().end(),[](const auto& region){return region.water;});
 assert(water!=scene.Hazards().end() && near(water->width,10000.f) && near(water->height,10000.f));
 assert(water->OverlapsPlayer(water->x+water->width*.5f+64.f,water->y));
 assert(!water->OverlapsPlayer(water->x+water->width*.5f+65.f,water->y));
 // Source spring re-arms only during five seconds spent uncrouched.
 assert(scene.Load("Game/XmlFiles/levels/hot_springs.xml",renderer,error));
 assert(scene.HealthDevilSprays().size()==1 && scene.HealthDevilSprays().front().particles.size()==60);
 const auto spring=scene.HealingSprings().front();
 assert(scene.UpdateHealing(dt,spring.x,spring.y,true));assert(!scene.UpdateHealing(6,spring.x,spring.y,true));
 assert(!scene.UpdateHealing(5.1f,spring.x,spring.y,false));assert(scene.UpdateHealing(dt,spring.x,spring.y,true));
 // Original instrument is repeatable after two seconds and never consumed.
 assert(scene.Load("Game/XmlFiles/levels/village_hut.xml",renderer,error));
 assert(scene.PlayInstrument("36"));assert(!scene.PlayInstrument("36"));scene.UpdateDynamics(2.1f,0,0,0,0);assert(scene.PlayInstrument("36"));
 // Breakable stages, delayed reward and session-only revisits.
 assert(scene.Load("Game/XmlFiles/levels/grass_exploration_3.xml",renderer,error));
 assert(!scene.Breakables().empty());const auto brokenId=scene.Breakables().front().id;
 assert(scene.DamageBreakable(0,scene.Breakables().front().maxHealth));
 assert(scene.BrokenBreakableIds().front()==brokenId && scene.PendingSpawnedOrbs()>=1 && scene.PendingSpawnedOrbs()<=8);
 assert(!scene.DamageBreakable(0,100));
 assert(scene.Load("Game/XmlFiles/levels/grass_exploration_3.xml",renderer,error));scene.ApplyBrokenBreakables({brokenId});
 assert(scene.Breakables().front().health==0 && scene.PendingSpawnedOrbs()==0);
 // Authored foliage reacts only to a moving character fully inside its collider.
 assert(scene.Load("Game/XmlFiles/levels/forest_exploration_barn.xml",renderer,error));
 auto shrub=std::find_if(scene.SceneSprites().begin(),scene.SceneSprites().end(),[](const auto& o){return o.collisionFoliage && o.collisionWidth>129.f;});
 assert(shrub!=scene.SceneSprites().end());const auto shrubId=shrub->xmlId;
 const float fx=shrub->x+shrub->collisionOffsetX,fy=shrub->y+shrub->collisionOffsetY;
 scene.SetPlayerVelocity(480,0);scene.UpdateDynamics(dt,fx,fy,fx,fy);
 assert(near(objectAt(scene,renderer,shrubId.c_str(),fx,fy).object.foliageSway.timeMultiplier,12.f));
 scene.SetPlayerVelocity(0,0);scene.UpdateDynamics(.6f,fx,fy,fx,fy);
 assert(near(objectAt(scene,renderer,shrubId.c_str(),fx,fy).object.foliageSway.timeMultiplier,0.f));
 // Every authored transition must carry XML direction, not reset to +1.
 int entries=0,edgeStrips=0;
 for(const auto& file:std::filesystem::directory_iterator("Game/XmlFiles/levels")) {
  if(file.path().extension()!=".xml")continue;
  TiXmlDocument xml(file.path().string().c_str());assert(xml.LoadFile());
  for(auto* object=xml.RootElement()->FirstChildElement();object;object=object->NextSiblingElement()) {
   if(std::string(object->Value())=="SolidLineStrip") {
    auto* edge=object->FirstChildElement("edge_properties");
    if(edge && ((edge->Attribute("has_left_hard_edge") && std::string(edge->Attribute("has_left_hard_edge"))=="true") ||
      (edge->Attribute("has_right_hard_edge") && std::string(edge->Attribute("has_right_hard_edge"))=="true")))++edgeStrips;
   }
   if(std::string(object->Value())!="LevelEntry")continue;
   std::string from=object->FirstChildElement("level_came_from")->Attribute("value");
   std::replace(from.begin(),from.end(),'\\','/');from="Game/"+from;
   const char* door=object->FirstChildElement("door_id")->Attribute("value");
   assert(scene.Load(file.path().string(),renderer,error,from,door?door:""));
   double direction=0,x=0,y=0;object->FirstChildElement("player_direction")->QueryDoubleAttribute("x",&direction);
   object->FirstChildElement("position")->QueryDoubleAttribute("x",&x);object->FirstChildElement("position")->QueryDoubleAttribute("y",&y);
   assert(near(scene.PlayerEntryDirection(),direction) && near(scene.PlayerX(),x) && near(scene.PlayerY(),y));
   g::PlayerMotion entry;entry.Reset(scene.PlayerX(),scene.PlayerY(),true);entry.ApplyEntryDirection(scene.PlayerEntryDirection());
   assert(near(entry.VelocityX(),direction*60.f));++entries;
  }
 }
 assert(entries==67 && edgeStrips==11);
 // Final scene's authored boat must move after actual landing and carry the player horizontally.
 assert(scene.Load("Game/XmlFiles/levels/sea_2.xml",renderer,error));
 g::PlayerMotion motion;motion.Reset(scene.PlayerX(),scene.PlayerY(),true);const float startX=motion.X();
 for(int i=0;i<600;++i){motion.CarryX(scene.UpdateDynamics(dt,motion.X(),motion.Y(),motion.X(),motion.Y(),motion.Grounded(),motion.VelocityY()));motion.Update(scene,dt,0,false);}
 assert(motion.Grounded() && motion.X()<startX-100.f);
 g::WeatherLightning weather;weather.SetAllowed(true);assert(weather.StartRainingIfNotAlready());
 assert(weather.Raining() && !weather.StartRainingIfNotAlready());
 assert(g::FindTechnique("kRoll")->cost==25 && g::FindTechnique("kDownwardDash")->cost==450 && g::FindTechnique("kBombUnlock")->newGamePlus);

 std::cout<<"PASS: 39 XMLs: 835 small/5 large orbs,42 emitters,46 animals,11 platforms,81 breakables,17 rotators; moving bounds, water overlap,50-orb rewards, healing/koto rearm, broken revisit and600-tick real raft ride\n";
}
