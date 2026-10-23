#include "../game/Combat.h"
#include <cassert>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
using namespace deathleak;
namespace deathleak::game {
struct CombatBloodReboundProbe {
    static void Spawn(Combat& combat) { combat.SpawnPlayerBloodFlight(0.f, 10.f); combat.bloodFlights_.back().vy = -20.f; }
    static const auto& Blood(const Combat& combat) { return combat.bloodFlights_.front(); }
};
}
class Spy final : public render::Renderer {
public:
    std::map<std::string, render::TextureId> ids;
    std::vector<render::Sprite> sprites;
    bool Initialize(const char*,int,int,std::string&) override { return true; }
    render::TextureId LoadTexture(const std::string& path,std::string&) override {
        return ids.try_emplace(path,static_cast<render::TextureId>(ids.size()+1)).first->second;
    }
    void BeginFrame(render::Color) override {}
    void DrawSprite(const render::Sprite& sprite) override { sprites.push_back(sprite); }
    void FillRect(render::Rect,render::Color) override {}
    void ApplyScenePostprocess() override {}
    void EndFrame() override {}
    bool PollQuit() override { return false; }
    bool KeyDown(int) const override { return false; }
    render::MouseState Mouse() const override { return {}; }
    bool SaveFrame(const std::string&,std::string&) override { return false; }
};
int main() {
    Spy renderer;
    game::Combat combat;
    combat.SetArt(renderer);
    const auto path=std::filesystem::temp_directory_path()/"deathleak_blood_blocked_rebound_cpu.xml";
    {
        std::ofstream out(path);
        out << "<level><Player id=\"1\"><position x=\"0\" y=\"100\"/><dimensions width=\"100\" height=\"200\"/><texture filename=\"Media/UI/player_silhouette.png\"/></Player>";
        out << "<SolidLineStrip id=\"2\"><position x=\"0\" y=\"5\"/><dimensions width=\"100\" height=\"1\"/><material value=\"caverock\"/><points num_points=\"2\" x_0=\"-50\" y_0=\"0\" x_1=\"50\" y_1=\"0\"/></SolidLineStrip>";
        out << "</level>";
    }
    game::LevelScene scene;
    std::string error;
    assert(scene.Load(path.string(),renderer,error));
    game::CombatBloodReboundProbe::Spawn(combat);
    auto tick=[&](float dt) { combat.Update(scene,1000,1000,dt,false,false,true,false,1,renderer); };
    tick(.1f);
    assert(combat.PlayerBloodFlightCount()==1);
    const auto& impact=game::CombatBloodReboundProbe::Blood(combat);
    assert(impact.blocked && !impact.impacted && std::abs(impact.y-5.f)<.001f);
    assert(std::abs(std::hypot(impact.vx/2.f,impact.vy/6.f)-1.f)<.0001f);
    const float x=impact.x,y=impact.y,vx=impact.vx,vy=impact.vy;
    assert(combat.MaterialSprayCount()==1);
    const auto* spray=combat.MaterialSpray(0);
    assert(spray && spray->depth==3000 && spray->particles.size()==10);
    assert(spray->particles[0].directionY<=0.f); // zero blood launch; only the helper's spread remains
    tick(1.f/60.f);
    const auto& rebound=game::CombatBloodReboundProbe::Blood(combat);
    assert(std::abs(rebound.x-(x+vx))<.0001f);
    assert(std::abs(rebound.y-(y+vy-1.15f))<.0001f);
    assert(std::abs(rebound.rotation-.09f)<.0001f);
    std::vector<game::PainterItem> items;
    combat.AppendPainterItems(items,renderer,0,0);
    bool sawFlight=false;
    for (const auto& item:items) if(item.depth==3000) {
        renderer.sprites.clear(); item.draw();
        for (const auto& sprite:renderer.sprites)
            if (sprite.texture==renderer.ids.at("Game/Media/blood_projectile.png")) {
                sawFlight=true; assert(std::abs(sprite.rotationDegrees-.09f)<.0001f);
            }
    }
    assert(sawFlight);
    tick(.25f-1.f/60.f);
    assert(combat.PlayerBloodFlightCount()==1);
    tick(.001f);
    assert(combat.PlayerBloodFlightCount()==0);
    std::filesystem::remove(path);
    std::cout << "PASS: blocked blood rebounds, spins, emits material spray and retires at 0.25s\n";
}
