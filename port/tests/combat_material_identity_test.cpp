#include "../game/Combat.h"
#include "../linux/RealSceneTopologyCapture.h"
#include <cassert>
#include <set>
#include <vector>

struct CombatMaterialIdentityProbe {
    static void Run() {
        using namespace deathleak;
        struct Recorder : render::Renderer {
            std::vector<render::Sprite> drawn;
            bool Initialize(const char*,int,int,bool,std::string&) override { return true; }
            render::TextureId LoadTexture(const std::string&,std::string&) override { return 1; }
            void BeginFrame(render::Color) override {}
            void DrawSprite(const render::Sprite& s) override { drawn.push_back(s); }
            void FillRect(render::Rect,render::Color) override {}
            void ApplyScenePostprocess() override {}
            void EndFrame() override {}
            bool PollQuit() override { return false; }
            bool KeyDown(int) const override { return false; }
            render::MouseState Mouse() const override { return {}; }
            bool SaveFrame(const std::string&,std::string&) override { return false; }
        } renderer;
        game::Combat combat;
        combat.bloodParticleArt_=1;
        combat.SpawnNinjaMaterialSpray(0,0);
        combat.SpawnNinjaMaterialSpray(10,0);
        assert(combat.materialSprays_.size()==2);
        assert(combat.materialSprays_[0].stableIdentity!=combat.materialSprays_[1].stableIdentity);
        std::set<linux_port::TopologyKey> keys;
        std::vector<game::PainterItem> items;
        combat.AppendPainterItems(items,renderer,0,0,[&](const auto& spray,const auto& particle,const auto& sprite) {
            assert(renderer.drawn.back().texture==sprite.texture);
            auto key=linux_port::EmittedParticleKey(spray,particle);
            assert(key.object && key.generation==1);
            assert(keys.insert(key).second);
        });
        for (const auto& item:items) item.draw();
        assert(keys.size()==40 && renderer.drawn.size()==40);
        const auto old=keys;
        combat.materialSprays_.erase(combat.materialSprays_.begin());
        combat.SpawnNinjaMaterialSpray(20,0);
        keys.clear(); items.clear(); renderer.drawn.clear();
        combat.AppendPainterItems(items,renderer,0,0,[&](const auto& spray,const auto& particle,const auto&) {
            keys.insert(linux_port::EmittedParticleKey(spray,particle));
        });
        for (const auto& item:items) item.draw();
        assert(keys.size()==40);
        for (const auto& key:keys) if (key.object>>16==combat.materialSprays_.back().stableIdentity)
            assert(!old.count(key));
        // Projectile emissions use birth IDs, not mutable vector positions.
        combat.shots_.clear();
        game::Shot first{0,0,100,0,1,1,false};
        first.texture=7; first.stableIdentity=combat.nextShotIdentity_++;
        game::Shot second=first; second.x=20; second.stableIdentity=combat.nextShotIdentity_++;
        combat.shots_={first,second};
        std::set<std::uint64_t> shotKeys;
        auto collectShots=[&] {
            items.clear(); renderer.drawn.clear(); shotKeys.clear();
            combat.AppendPainterItems(items,renderer,0,0,{},[&](const auto& shot,const auto& sprite) {
                assert(renderer.drawn.back().texture==sprite.texture);
                assert(shotKeys.insert(shot.stableIdentity).second);
            });
            for (const auto& item:items) item.draw();
        };
        collectShots();
        assert(shotKeys.size()==2 && shotKeys.count(first.stableIdentity) && shotKeys.count(second.stableIdentity));
        combat.shots_.erase(combat.shots_.begin());
        game::Shot third=first; third.stableIdentity=combat.nextShotIdentity_++;
        combat.shots_.push_back(third);
        collectShots();
        assert(shotKeys.size()==2 && shotKeys.count(second.stableIdentity) && shotKeys.count(third.stableIdentity));
        assert(!shotKeys.count(first.stableIdentity));
        // A recycled looping slot must not pair with its previous generation.
        auto p=combat.materialSprays_.back().particles[0];
        const auto before=linux_port::EmittedParticleKey(combat.materialSprays_.back(),p);
        ++p.generation;
        assert(!(before==linux_port::EmittedParticleKey(combat.materialSprays_.back(),p)));
    }
};
int main() { CombatMaterialIdentityProbe::Run(); }
