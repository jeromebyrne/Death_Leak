#include "../game/Combat.h"
#include <cassert>
#include <cmath>
#include <iostream>
namespace deathleak::game {
struct BoundedAIProbe {
    struct Recorder final : render::Renderer {
        std::vector<render::Sprite> sprites;
        bool Initialize(const char*,int,int,bool,std::string&) override { return true; }
        render::TextureId LoadTexture(const std::string&,std::string&) override { return 1; }
        void BeginFrame(render::Color) override {}
        void DrawSprite(const render::Sprite& s) override { sprites.push_back(s); }
        void FillRect(render::Rect,render::Color) override {}
        void ApplyScenePostprocess() override {}
        void EndFrame() override {}
        bool PollQuit() override { return false; }
        bool KeyDown(int) const override { return false; }
        render::MouseState Mouse() const override { return {}; }
        bool SaveFrame(const std::string&,std::string&) override { return false; }
    } renderer;
    LevelScene empty;
    Enemy Target(float x=0,float y=0,const char* type="kNinja") {
        Enemy e{x,y,1000,0,type,1};e.jumpRollDelay=100;e.teleportCooldown=100;return e;
    }
    void Tick(Combat& c,float x,float y,bool ground=true,const LevelScene* scene=nullptr) {
        c.Update(scene?*scene:empty,x,y,1.f/60.f,false,false,ground,false,1,renderer);
    }
    void Run() {
        Combat close;close.enemies_.push_back(Target());Tick(close,100,0);
        assert(close.enemies_[0].x<0 && close.enemies_[0].windup==0 && close.shots_.empty());
        Combat far;far.enemies_.push_back(Target());Tick(far,650,0);
        assert(far.enemies_[0].x>0 && far.enemies_[0].windup==0 && far.shots_.empty());
        Combat vertical;vertical.enemies_.push_back(Target());Tick(vertical,100,650,false);
        assert(vertical.enemies_[0].x>0 && vertical.enemies_[0].windup==0);
        Combat band;band.enemies_.push_back(Target());Tick(band,350,0);Tick(band,350,0);
        assert(band.enemies_[0].windup>0);float bx=band.enemies_[0].x;
        for(int i=0;i<10;++i){Tick(band,100,0);assert(band.enemies_[0].x==bx);}
        assert(band.shots_.empty());
        for(int i=0;i<10;++i)Tick(band,100,0);
        assert(!band.shots_.empty());
        Combat above;above.playerGroundTime_=1.1f;above.enemies_.push_back(Target(0,-550));
        above.enemies_[0].teleportCooldown=0;above.SetEncounterView(0,0,1,-480);
        Tick(above,0,0);assert(above.enemies_[0].teleportTime>0 && above.enemies_[0].teleportX==-550);
        Combat off;off.enemies_.push_back(Target());off.enemies_[0].teleportCooldown=0;
        off.SetEncounterView(-5000,0,1,480);Tick(off,650,0);
        assert(off.enemies_[0].teleportTime==0);
        Combat fly;auto g=Target(300,300,"kGhost");g.targetY=1000;g.ghostTargetAge=2.1f;fly.enemies_.push_back(g);
        fly.SetEncounterView(0,0,1,0);Tick(fly,0,100,false);
        assert(fly.enemies_[0].targetY==1000 && fly.enemies_[0].y==300);
        Tick(fly,0,100,true);assert(fly.enemies_[0].targetY==370);
        assert(fly.enemies_[0].y>300 && fly.enemies_[0].y<301);
        LevelScene central;std::string error;assert(central.Load("Game/XmlFiles/levels/village_central.xml",renderer,error));
        size_t support=size_t(-1);float px=0,py=0;
        for(size_t i=0;i<central.Ground().size();++i){const auto& l=central.Ground()[i];
            if(l.hardRight && std::abs(l.x2-l.x1)>1 && l.stripRight-l.rightOffset>=std::min(l.x1,l.x2) && l.stripRight-l.rightOffset<=std::max(l.x1,l.x2)) {
                support=i;px=l.stripRight-l.rightOffset-1;py=l.y1+(px-l.x1)*(l.y2-l.y1)/(l.x2-l.x1)+100;break;
            }
        }
        assert(support!=size_t(-1));Combat edge;auto e=Target(px,py);e.grounded=true;e.supportingSegment=support;edge.enemies_.push_back(e);
        Tick(edge,px+550,py,true,&central);assert(edge.enemies_[0].x==px);
        // Actual Combat dispatcher: butterflies continue both ownership kinds;
        // rabbit NPCs reject hostile shots and retire only on friendly damage.
        LevelScene animals;assert(animals.Load("Game/XmlFiles/levels/hot_springs.xml",renderer,error));
        auto butterfly=animals.AmbientTargets().front();Combat butterflyShot;
        Shot blade{butterfly.x+butterfly.offsetX,butterfly.y+butterfly.offsetY,0,0,120,1,true};
        blade.gravity=false;blade.npcProjectile=true;blade.collisionWidth=blade.collisionHeight=1;butterflyShot.shots_.push_back(blade);
        butterflyShot.Update(animals,10000,10000,0,false,false,true,false,1,renderer,0,0,{}, {},
            [&](size_t index,float damage){animals.DamageAmbient(index,damage,renderer);});
        assert(animals.AmbientTargets().size()==1 && butterflyShot.shots_.size()==1 && butterflyShot.shots_[0].active);
        assert(animals.AmbientDebrisBodies().size()==2);
        assert(animals.Load("Game/XmlFiles/levels/catacombs_entrance_1.xml",renderer,error));
        auto rabbit=animals.AmbientTargets().front();Combat rabbitShot;
        blade.x=rabbit.x+rabbit.offsetX;blade.y=rabbit.y+rabbit.offsetY;rabbitShot.shots_.push_back(blade);
        rabbitShot.Update(animals,10000,10000,0,false,false,true,false,1,renderer,0,0,{}, {},
            [&](size_t index,float damage){animals.DamageAmbient(index,damage,renderer);});
        assert(animals.AmbientTargets().size()==2 && rabbitShot.shots_[0].active);
        rabbitShot.shots_[0].enemy=false;rabbitShot.shots_[0].inWater=true;
        rabbitShot.Update(animals,10000,10000,0,false,false,true,false,1,renderer,0,0,{}, {},
            [&](size_t index,float damage){animals.DamageAmbient(index,damage,renderer);});
        assert(animals.AmbientTargets().size()==2 && !rabbitShot.shots_[0].active);
        rabbitShot.shots_[0].active=true;rabbitShot.shots_[0].inWater=false;

        rabbitShot.Update(animals,10000,10000,0,false,false,true,false,1,renderer,0,0,{}, {},
            [&](size_t index,float damage){animals.DamageAmbient(index,damage,renderer);});
        assert(animals.AmbientTargets().size()==1 && !rabbitShot.shots_[0].active && animals.PendingSpawnedOrbs()==0);
        Combat blast;blast.SetEncounterView(0,0,.5f,0);blast.enemies_.push_back(Target(100,0));blast.enemies_.push_back(Target(900,0));
        Shot shot{100,0,0,0,100,1,false};shot.gravity=false;blast.shots_.push_back(shot);
        shot.x=900;blast.shots_.push_back(shot);shot.x=1100;blast.shots_.push_back(shot);
        Combat::BloodFlight blood{};blood.x=100;blood.size=10;blast.bloodFlights_.push_back(blood);
        Combat::DeathEffect debris{};debris.x=0;debris.y=100;debris.width=debris.height=10;blast.deathEffects_.push_back(debris);
        const float health=blast.Health();blast.ApplyExplosion(0,0,1000,500);
        assert(blast.enemies_[0].health==500 && blast.enemies_[1].health==1000);
        assert(blast.shots_[0].vx==600 && blast.shots_[1].vx==0 && blast.shots_[2].vx==0);
        assert(blast.bloodFlights_[0].vx==10 && blast.deathEffects_[0].vy==900 && blast.Health()==health);
        std::cout<<"PASS: bounded NPC windup/XY-band, vertical/offscreen teleport, ghost target hold/camera/inertia, supporting-strip hard edge; animal ownership/continuation; typed visible explosion impulses\n";
    }
};
}
int main(){deathleak::game::BoundedAIProbe{}.Run();}
