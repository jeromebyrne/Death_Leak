#include "../game/Combat.h"
#include <cassert>
#include <iostream>

struct EncounterCooldownProbe {
    using Combat = deathleak::game::Combat;
    struct Recorder : deathleak::render::Renderer {
        bool Initialize(const char*,int,int,bool,std::string&) override { return true; }
        deathleak::render::TextureId LoadTexture(const std::string&,std::string&) override { return 1; }
        void BeginFrame(deathleak::render::Color) override {}
        void DrawSprite(const deathleak::render::Sprite&) override {}
        void FillRect(deathleak::render::Rect,deathleak::render::Color) override {}
        void ApplyScenePostprocess() override {}
        void EndFrame() override {}
        bool PollQuit() override { return false; }
        bool KeyDown(int) const override { return false; }
        deathleak::render::MouseState Mouse() const override { return {}; }
        bool SaveFrame(const std::string&,std::string&) override { return false; }
    } renderer;
    static constexpr std::int64_t lastSpawn = 1000000;
    static void Clock(Combat& combat, std::int64_t now) {
        combat.simulatedWallClock_ = true;
        combat.simulatedClockOriginMs_ = now;
        combat.simulatedClockElapsedMs_ = 0;
    }
    void Run() {
        deathleak::game::LevelScene scene;
        std::string error;
        assert(scene.Load("port/tests/fixtures/encounter_open.xml",renderer,error));
        assert(scene.Encounters().size()==1);
        const auto id = scene.Encounters().front().id;
        auto tick = [&](Combat& c, bool overlap) {
            c.Update(scene,overlap?0.f:10000.f,107.44937f,1.f/60.f,
                     false,false,true,false,1,renderer);
        };
        Combat cooling;
        cooling.SetTriggerHistory({{id,lastSpawn}});
        Clock(cooling,lastSpawn+119999);
        cooling.InitializeLevelEncounters(scene); // source removes it here
        tick(cooling,false); // threshold expires while outside its rectangle
        cooling.InitializeLevelEncounters(scene); // cannot resurrect this visit
        tick(cooling,true);
        assert(cooling.Spawned()==0 && cooling.EnemyCount()==0);
        assert(cooling.TriggerHistory().at(id)==lastSpawn); // suppressed != fired
        cooling.ClearLevel(); // a real new visit after expiry may respawn
        cooling.InitializeLevelEncounters(scene);
        tick(cooling,true);
        assert(cooling.Spawned()==1 && cooling.EnemyCount()==1);
        assert(cooling.TriggerHistory().at(id)>lastSpawn+120000);
        const auto fired = cooling.TriggerHistory().at(id);
        Clock(cooling,fired+130000);
        tick(cooling,true); // a fired trigger stays consumed even after expiry
        assert(cooling.Spawned()==1 && cooling.TriggerHistory().at(id)==fired);
        cooling.ClearLevel();
        cooling.InitializeLevelEncounters(scene);
        tick(cooling,true);
        assert(cooling.Spawned()==2); // do not disable legitimate later respawns

        Combat boundary;
        boundary.SetTriggerHistory({{id,lastSpawn}});
        Clock(boundary,lastSpawn+120000); // native strict <, not <=
        boundary.InitializeLevelEncounters(scene);
        tick(boundary,true);
        assert(boundary.Spawned()==1);

        Combat future;
        future.SetTriggerHistory({{id,lastSpawn+1000}});
        Clock(future,lastSpawn);
        future.InitializeLevelEncounters(scene);
        Clock(future,lastSpawn+200000);
        tick(future,true);
        assert(future.Spawned()==0); // negative elapsed also removed at load
        future.ClearLevel();
        future.InitializeLevelEncounters(scene);
        tick(future,true);
        assert(future.Spawned()==1);

        Combat fresh;
        Clock(fresh,lastSpawn);
        tick(fresh,true); // missing history / CPU caller lazy initialization
        assert(fresh.Spawned()==1);
        std::cout << "PASS: cooldown latched at level install, expiry cannot resurrect, "
                     "new-visit respawn, exact120s boundary, future history, "
                     "once-per-visit and missing history\n";
    }
};
int main() { EncounterCooldownProbe{}.Run(); }
