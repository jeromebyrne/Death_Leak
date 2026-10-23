#include "../game/Combat.h"
#include <cassert>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
using namespace deathleak;
namespace deathleak::game {
struct CombatBloodOcclusionProbe {
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
    const auto path=std::filesystem::temp_directory_path()/"deathleak_blood_first_line_occlusion_cpu.xml";
    auto sceneWith=[&](const char* frontMaterial, const char* rearMaterial) {
        std::ofstream out(path);
        out << "<level><Player id=\"1\"><position x=\"0\" y=\"100\"/><dimensions width=\"100\" height=\"200\"/><texture filename=\"Media/UI/player_silhouette.png\"/></Player>";
        // Rear line is authored first: winner must be geometric, not XML order.
        for (int i=0;i<2;++i) {
            const int y=i ? 5 : 0;
            const char* material=i ? frontMaterial : rearMaterial;
            out << "<SolidLineStrip id=\"" << (i+2) << "\"><position x=\"0\" y=\"" << y
                << "\"/><dimensions width=\"100\" height=\"1\"/>";
            if (*material) out << "<material value=\"" << material << "\"/>";
            out << "<points num_points=\"2\" x_0=\"-50\" y_0=\"0\" x_1=\"50\" y_1=\"0\"/>"
                << "</SolidLineStrip>";
        }
        out << "</level>";
        out.close();
        game::LevelScene scene;
        std::string error;
        assert(scene.Load(path.string(),renderer,error));
        assert(scene.Ground().size()==2);
        return scene;
    };
    auto tick=[&](const game::LevelScene& scene,float dt) { combat.Update(scene,1000,1000,dt,false,false,true,false,1,renderer); };
    auto bloodVisual=[&]() {
        std::vector<game::PainterItem> items;
        combat.AppendPainterItems(items,renderer,0,0);
        int depth=-1;
        render::TextureId texture=0;
        for (const auto& item:items) if (item.depth==3000 || item.depth==4000) {
            renderer.sprites.clear(); item.draw();
            for (const auto& sprite:renderer.sprites) {
                if (sprite.texture==renderer.ids.at("Game/Media/blood_projectile.png") ||
                    sprite.texture==renderer.ids.at("Game/Media/blood_impact.png") ||
                    sprite.texture==renderer.ids.at("Game/Media/blood_impact_2.png")) {
                    depth=item.depth; texture=sprite.texture;
                }
            }
        }
        return std::pair{depth,texture};
    };
    for (const char* blocker : {"caverock", "unknown_material"}) {
        combat.ClearLevel();
        auto scene=sceneWith(blocker,"water");
        game::CombatBloodOcclusionProbe::Spawn(combat);
        tick(scene,.1f); // crosses y=5 and y=0 in one sweep
        const auto& blood=game::CombatBloodOcclusionProbe::Blood(combat);
        assert(!blood.impacted && blood.blocked);
        assert(std::abs(blood.y-5.f)<.001f); // first contact is front, not rear
        assert(std::abs(std::hypot(blood.vx/2.f,blood.vy/6.f)-1.f)<.0001f);
        const float x=blood.x, y=blood.y, vx=blood.vx, vy=blood.vy;
        const size_t expectedSprays=std::string(blocker)=="caverock" ? 1u : 0u;
        assert(combat.MaterialSprayCount()==expectedSprays);
        if (expectedSprays) {
            const auto* spray=combat.MaterialSpray(0);
            assert(spray && spray->depth==3000 && spray->particles.size()==10);
        }
        assert((bloodVisual()==std::pair{3000,renderer.ids.at("Game/Media/blood_projectile.png")}));
        tick(scene,.1f);
        const auto& moved=game::CombatBloodOcclusionProbe::Blood(combat);
        assert(moved.blocked && !moved.impacted);
        assert(std::abs(moved.x-(x+2.f*vx))<.0001f);
        assert(std::abs(moved.y-(y+2.f*(vy-1.15f)))<.0001f);
        assert(std::abs(moved.rotation-.09f)<.0001f);
        assert(combat.MaterialSprayCount()==expectedSprays); // no rear-line spray on later ticks
        assert((bloodVisual()==std::pair{3000,renderer.ids.at("Game/Media/blood_projectile.png")}));
        tick(scene,.15f); // exact 0.25 s blocked lifetime survives
        assert(combat.PlayerBloodFlightCount()==1);
        assert(game::CombatBloodOcclusionProbe::Blood(combat).blocked);
        assert(combat.MaterialSprayCount()==expectedSprays);
        tick(scene,.001f);
        assert(combat.PlayerBloodFlightCount()==0);
    }
    combat.ClearLevel();
    auto scene=sceneWith("water","caverock");
    game::CombatBloodOcclusionProbe::Spawn(combat);
    tick(scene,.1f);
    const auto& blood=game::CombatBloodOcclusionProbe::Blood(combat);
    assert(blood.impacted && !blood.blocked && combat.MaterialSprayCount()==1);
    assert(bloodVisual().first==4000);
    std::filesystem::remove(path);
    std::cout << "PASS: nearest blood line rebounds without rear splat; pierceable front still splats\n";
}
