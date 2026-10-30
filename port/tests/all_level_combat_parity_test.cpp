#include "../game/Combat.h"
#include <cassert>
#include <cmath>
#include <iostream>

namespace deathleak::game {
struct AllLevelCombatParityProbe {
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
    static bool Near(float a,float b) { return std::abs(a-b)<.002f; }
    void Tick(Combat& c,float dt,float x=10000.f,float y=10000.f,const LevelScene* scene=nullptr) {
        c.Update(scene?*scene:empty,x,y,dt,false,false,true,false,1,renderer);
    }
    void Draw(Combat& c) { renderer.sprites.clear(); c.Draw(renderer,0,0); }
    Enemy Target(const std::string& type,float x=0,float y=0) {
        Enemy e{x,y,1000,1000,type,1}; e.stunTime=100; return e;
    }
    Shot NpcShot(float x,float y,float vx,float vy,float width=25,float height=13) {
        Shot s{x,y,vx,vy,120,3,true}; s.npcProjectile=true;
        s.collisionWidth=width; s.collisionHeight=height; s.gravity=false;
        s.texture=1; s.visualWidth=90; s.visualHeight=90; return s;
    }
    bool EnemyHit(const std::string& type,float sx,float sy) {
        Combat c; c.enemies_.push_back(Target(type));
        Shot s{sx,sy,0,0,10,1.5f,false}; s.gravity=false;
        c.shots_.push_back(s); Tick(c,0);
        return c.enemies_[0].health<1000;
    }
    void Run() {
        // Source Focus reload consumes five times the modified delta and
        // bypasses the burst gate without advancing NPC/world clocks faster.
        Combat focus;focus.fireCooldown_=.22f;focus.burstCount_=5;focus.burstCooldown_=.5f;
        focus.SetFocusing(true);Tick(focus,.01f);
        assert(Near(focus.fireCooldown_,.17f) && Near(focus.burstCooldown_,.5f) && focus.burstCount_==5);
        focus.SetFocusing(false);Tick(focus,.01f);
        assert(focus.burstCount_==0 && Near(focus.burstCooldown_,.5f));
        // NPCTrigger::IsSolidSpriteInRectangle: strict intersection, not center.
        LevelScene encounter; std::string error;
        assert(encounter.Load("port/tests/fixtures/encounter_open.xml",renderer,error));
        const auto& t=encounter.Encounters().front();
        const float edge=t.x+t.width/2+128.959793f/2;
        Combat outside; Tick(outside,0,edge,t.y,&encounter); assert(outside.Spawned()==0);
        Combat overlap; Tick(overlap,0,edge-.01f,t.y,&encounter); assert(overlap.Spawned()==1);
        Combat above; Tick(above,0,t.x,t.y+t.height/2+214.898743f/2-.01f,&encounter);
        assert(above.Spawned()==1);
        Draw(overlap); assert(renderer.sprites.size()==1 && !renderer.sprites[0].flipY);
        // The other source spawners perform the same FlipVertical().
        for (const auto* type : {"kGhost","kSkeleton"}) {
            Combat c; c.enemies_.push_back(Target(type)); Draw(c);
            assert(renderer.sprites.size()==1 && !renderer.sprites[0].flipY);
        }
        // Actual runtime birth path: PNG IHDR 90x90 and 100x83; skeleton
        // explicitly overrides native sizing to 95.625x62.625. Collider facts
        // are independently pinned to NPC/GhostEnemy/SkeletonEnemy constructors.
        for (const auto* type : {"kNinja","kGhost","kSkeleton"}) {
            Combat c; c.SetArt(renderer);
            auto e=Target(type,500,300); e.stunTime=0; e.fireCooldown=0;
            e.windup=.001f; e.targetX=500; e.targetY=300; c.enemies_.push_back(e);
            Tick(c,.01f,500,0); assert(c.shots_.size()==1);
            const auto& s=c.shots_[0]; const bool ghost=std::string(type)=="kGhost";
            const bool skeleton=std::string(type)=="kSkeleton";
            assert(Near(s.visualWidth,skeleton?95.625f:ghost?100.f:90.f));
            assert(Near(s.visualHeight,skeleton?62.625f:ghost?83.f:90.f));
            assert(Near(s.collisionWidth,ghost?40.f:25.f));
            assert(Near(s.collisionHeight,skeleton?8.f:ghost?25.f:13.f));
            assert(Near(s.inactiveRetention,ghost||skeleton?2.f:.25f));
            assert(s.npcProjectile && s.active && s.life>100.f);
            c.enemies_.clear(); Tick(c,2.1f); assert(c.shots_.size()==1 && c.shots_[0].active);
        }
        // Source active fade .3s, -20 rad/s star spin, vertical-only left flip.
        for (int direction : {-1,1}) {
            Combat c; auto s=NpcShot(0,0,direction*540.f,0);
            s.spinning=true; s.authoredFlipY=direction<0; c.shots_.push_back(s);
            Tick(c,.15f); Draw(c); assert(renderer.sprites.size()==1);
            auto v=renderer.sprites[0];
            assert(Near(v.destination.width,30.f) && Near(v.destination.height,30.f));
            assert(v.tint.a==128 && !v.flipX && v.flipY==(direction>0));
            const float expected=direction>0?171.88734f:-171.88734f;
            assert(Near(v.rotationDegrees,expected));
            Tick(c,.15f); Draw(c); assert(renderer.sprites[0].tint.a==255);
            assert(Near(renderer.sprites[0].destination.width,60.f));
        }
        // A star can actually reach the player beyond the old .25s/135-unit cap.
        Combat reach; reach.shots_.push_back(NpcShot(-500,0,540,0));
        for(int i=0;i<60 && reach.Hits()==0;++i) Tick(reach,1.f/60,0,0);
        assert(reach.Hits()==1 && Near(reach.Health(),22.f));
        assert(reach.shots_.size()==1 && !reach.shots_[0].active);
        Tick(reach,.125f,0,0); Draw(reach);
        // Blood is also drawn, so identify the retained shot through callback.
        render::Sprite retained; int seen=0; std::vector<PainterItem> items;
        reach.AppendPainterItems(items,renderer,0,0,{},[&](const Shot&,const render::Sprite& v){ retained=v; ++seen; });
        for(auto& item:items) item.draw();
        assert(seen==1 && retained.tint.a==128);
        Tick(reach,.126f,0,0); assert(reach.shots_.empty());
        // Liquid inactive retirement is two seconds, independent of its birth
        // age; its pitched rotation survives the collision (only stars reset).
        Combat liquid; auto vomit=NpcShot(-1,0,-1,-1,40,25);
        vomit.resistance=.9f; vomit.visualWidth=100; vomit.visualHeight=83;
        vomit.inactiveRetention=2; vomit.activeVisualAge=3;
        liquid.shots_.push_back(vomit); Tick(liquid,0,0,0);
        assert(!liquid.shots_[0].active && liquid.shots_[0].authoredFlipY);
        assert(Near(liquid.shots_[0].rotation,2.35619449f));
        Tick(liquid,1,10000,10000); assert(liquid.shots_.size()==1);
        Tick(liquid,1.01f); assert(liquid.shots_.empty());
        // Exact expanded pair AABBs: player 128.959793x214.898743,
        // ninja star 25x13; old point-vs-70x180 misses both these contacts.
        for (const auto& p : {std::pair<float,float>{76.9f,0.f},{0.f,113.9f}}) {
            Combat c; c.shots_.push_back(NpcShot(p.first,p.second,0,0)); Tick(c,0,0,0);
            assert(c.Hits()==1);
        }
        Combat miss; miss.shots_.push_back(NpcShot(77.f,0,0,0)); Tick(miss,0,0,0); assert(miss.Hits()==0);
        // Player knife 84.5x25; ninja collider100x200; ghost55x130 +Y20.
        assert(EnemyHit("kNinja",92.f,0)); assert(!EnemyHit("kNinja",92.3f,0));
        assert(EnemyHit("kGhost",69.7f,20)); assert(!EnemyHit("kGhost",69.8f,20));
        assert(EnemyHit("kGhost",0,97.4f)); assert(!EnemyHit("kGhost",0,97.6f));
        assert(EnemyHit("kGhost",0,-57.4f)); assert(!EnemyHit("kGhost",0,-57.6f));
        // Swept corner crossing (closest point projection cannot represent
        // independent rectangular extents) and terrain occlusion remain real.
        Combat sweep; sweep.shots_.push_back(NpcShot(-200,110,2000,0)); Tick(sweep,.2f,0,0); assert(sweep.Hits()==1);
        LevelScene solid; assert(solid.Load("port/tests/fixtures/solid_box.xml",renderer,error));
        Combat blocked; blocked.shots_.push_back(NpcShot(10,200,1200,0));
        Tick(blocked,.2f,240,200,&solid); assert(blocked.Hits()==0 && !blocked.shots_[0].active);
        Combat beforeWall; beforeWall.shots_.push_back(NpcShot(10,200,1200,0));
        Tick(beforeWall,.2f,100,200,&solid); assert(beforeWall.Hits()==1);
        // Legitimate hurt cooldown is not replaced by invulnerability or grants.
        Combat cooldown; cooldown.shots_.push_back(NpcShot(0,0,0,0));
        cooldown.shots_.push_back(NpcShot(0,0,0,0)); Tick(cooldown,0,0,0);
        assert(cooldown.Hits()==1 && Near(cooldown.Health(),22));
        Tick(cooldown,.51f,0,0); assert(cooldown.Hits()==2 && Near(cooldown.Health(),19));
        std::cout << "PASS all-level combat: source V mapping, runtime native/explicit sizing, active/inactive lifetime, fade/spin/reflection, strict triggers, typed swept hit extents, terrain and hurt cooldown\n";
    }
};
}
int main() { deathleak::game::AllLevelCombatParityProbe{}.Run(); }
