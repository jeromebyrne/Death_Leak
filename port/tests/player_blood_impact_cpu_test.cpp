#include "../game/Combat.h"
#include <cassert>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
using namespace deathleak;
namespace deathleak::game {
struct CombatBloodFlightProbe {
    static void Spawn(Combat& combat, float x, float y, float vy) {
        combat.SpawnPlayerBloodFlight(x,y);
        combat.bloodFlights_.back().vy=vy;
    }
    static const auto& Blood(const Combat& combat) { return combat.bloodFlights_.front(); }
    static void SetInactiveAge(Combat& combat,float age) { combat.bloodFlights_.front().inactiveAge=age; }
    static const auto& Sprays(const Combat& combat) { return combat.materialSprays_; }
    static void EnemyAlive(Combat& combat) { combat.enemies_.push_back({1000,1000,1,0,"Ninja",1}); }
};
}
class Spy final : public render::Renderer {
public:
    std::map<std::string,render::TextureId> ids;
    std::vector<render::Sprite> sprites;
    bool Initialize(const char*,int,int,std::string&) override {return true;}
    render::TextureId LoadTexture(const std::string& path,std::string&) override {
        auto [it,created]=ids.try_emplace(path,static_cast<render::TextureId>(ids.size()+1)); return it->second;
    }
    void BeginFrame(render::Color) override {}
    void DrawSprite(const render::Sprite& s) override {sprites.push_back(s);}
    void FillRect(render::Rect,render::Color) override {}
    void ApplyScenePostprocess() override {}
    void EndFrame() override {}
    bool PollQuit() override {return false;}
    bool KeyDown(int) const override {return false;}
    render::MouseState Mouse() const override {return {};}
    bool SaveFrame(const std::string&,std::string&) override {return false;}
};
int main() {
    Spy renderer;
    game::Combat combat;
    combat.SetArt(renderer);
    auto path=std::filesystem::temp_directory_path()/"deathleak_blood_impact_cpu.xml";
    auto sceneWith=[&](const char* material) {
        std::ofstream out(path);
        out << "<level><Player id=\"1\"><position x=\"0\" y=\"100\"/><dimensions width=\"100\" height=\"200\"/><texture filename=\"Media/UI/player_silhouette.png\"/></Player>"
            << "<SolidLineStrip id=\"2\"><position x=\"0\" y=\"0\"/><dimensions width=\"100\" height=\"1\"/>";
        if (*material) out << "<material value=\"" << material << "\"/>";
        out << "<points num_points=\"2\" x_0=\"-50\" y_0=\"0\" x_1=\"50\" y_1=\"0\"/>"
            << "</SolidLineStrip></level>";
        out.close();
        game::LevelScene scene;
        std::string error;
        assert(scene.Load(path.string(),renderer,error));
        assert(scene.Ground().size()==1);
        return scene;
    };
    auto tick=[&](const game::LevelScene& scene,float dt) {combat.Update(scene,1000,1000,dt,false,false,true,false,1,renderer);};
    auto visual=[&](int expectedDepth) {
        std::vector<game::PainterItem> items;
        combat.AppendPainterItems(items,renderer,0,0);
        for (const auto& item:items) if (item.depth==expectedDepth) {
            renderer.sprites.clear(); item.draw();
            if (!renderer.sprites.empty()) return renderer.sprites.back();
        }
        assert(false); return render::Sprite{};
    };
    auto water=sceneWith("water");
    game::CombatBloodFlightProbe::EnemyAlive(combat); // knife would skip the line
    game::CombatBloodFlightProbe::Spawn(combat,0,10,-20);
    tick(water,.1f);
    const auto& b=game::CombatBloodFlightProbe::Blood(combat);
    assert(b.impacted && b.vy==0 && b.inactiveAge==0);
    assert(b.y>=-39.8f && b.y<=-.2f); // signed 0..19.8 jitter plus -20
    assert(combat.Health()==25 && combat.EnemyCount()==1);
    assert(combat.MaterialSprayCount()==1);
    assert(game::CombatBloodFlightProbe::Sprays(combat)[0].particles.size()==10);
    assert(game::CombatBloodFlightProbe::Sprays(combat)[0].depth==3000);
    assert(std::abs(game::CombatBloodFlightProbe::Sprays(combat)[0].y-(-32.f))<.01f);
    auto atImpact=visual(4000);
    assert(atImpact.texture==renderer.ids.at(b.variant ? "Game/Media/blood_impact_2.png":"Game/Media/blood_impact.png"));
    assert(atImpact.destination.width==0 && atImpact.tint.a==255);
    const float impactY=b.y;
    tick(water,.275f);
    assert(game::CombatBloodFlightProbe::Blood(combat).y==impactY);
    assert(visual(4000).destination.width>0);
    tick(water,4.675f); // 90% elapsed: alpha half
    assert(std::abs(static_cast<int>(visual(4000).tint.a)-128)<=1);
    game::CombatBloodFlightProbe::SetInactiveAge(combat,5.5f);
    tick(water,0.f); // equality survives
    assert(combat.PlayerBloodFlightCount()==1);
    tick(water,.001f);
    assert(combat.PlayerBloodFlightCount()==0);
    combat.ClearLevel();
    auto stone=sceneWith("caverock");
    game::CombatBloodFlightProbe::Spawn(combat,0,10,-20);
    tick(stone,.1f);
    const auto& blocked=game::CombatBloodFlightProbe::Blood(combat);
    assert(blocked.blocked && !blocked.impacted); // nonpierceable line rebounds without splatting
    assert(std::abs(std::hypot(blocked.vx/2.f,blocked.vy/6.f)-1.f)<.0001f);
    assert(combat.MaterialSprayCount()==1);
    assert(game::CombatBloodFlightProbe::Sprays(combat)[0].particles.size()==10);
    assert(game::CombatBloodFlightProbe::Sprays(combat)[0].depth==3000);
    std::vector<game::PainterItem> blockedItems;
    combat.AppendPainterItems(blockedItems,renderer,0,0);
    bool sawFlight=false;
    for (const auto& item:blockedItems) if (item.depth==3000) {
        renderer.sprites.clear(); item.draw();
        for (const auto& sprite:renderer.sprites)
            sawFlight |= sprite.texture==renderer.ids.at("Game/Media/blood_projectile.png");
    }
    assert(sawFlight);
    combat.ClearLevel();
    auto bare=sceneWith("");
    game::CombatBloodFlightProbe::Spawn(combat,0,10,-20);
    tick(bare,.1f);
    assert(game::CombatBloodFlightProbe::Blood(combat).impacted && combat.MaterialSprayCount()==0);
    combat.ClearLevel();
    assert(combat.PlayerBloodFlightCount()==0);
    std::filesystem::remove(path);
    std::cout << "PASS: first blood line splat, material/no-material, inactive envelope, nonpierceable rebound\n";
}
