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
    static const auto& Blood(const Combat& combat) { return combat.bloodFlights_.front(); }
    static void SpawnOrdinary(Combat& combat, float x, float y, float vy) {
        combat.SpawnPlayerBloodFlight(x, y);
        combat.bloodFlights_.back().vy = vy;
    }
    static void SetRandomState(Combat& combat, std::uint32_t state) { combat.bloodRandomState_ = state; }
};
}
class Spy final : public render::Renderer {
public:
    std::map<std::string, render::TextureId> ids;
    bool Initialize(const char*, int, int, std::string&) override { return true; }
    render::TextureId LoadTexture(const std::string& path, std::string&) override {
        return ids.try_emplace(path, static_cast<render::TextureId>(ids.size()+1)).first->second;
    }
    void BeginFrame(render::Color) override {}
    void DrawSprite(const render::Sprite&) override {}
    void FillRect(render::Rect, render::Color) override {}
    void ApplyScenePostprocess() override {}
    void EndFrame() override {}
    bool PollQuit() override { return false; }
    bool KeyDown(int) const override { return false; }
    render::MouseState Mouse() const override { return {}; }
    bool SaveFrame(const std::string&, std::string&) override { return false; }
};
static bool Near(float a, float b) { return std::abs(a-b) < .001f; }
int main() {
    Spy renderer;
    game::Combat combat;
    combat.SetArt(renderer);
    const auto path = std::filesystem::temp_directory_path() / "deathleak_directed_blood_position_cpu.xml";
    auto sceneWithLine = [&](float x1, float y1, float x2, float y2) {
        std::ofstream out(path);
        out << "<level><Player id=\"1\"><position x=\"0\" y=\"100\"/><dimensions width=\"100\" height=\"200\"/><texture filename=\"Media/UI/player_silhouette.png\"/></Player>"
            << "<SolidLineStrip id=\"2\"><position x=\"0\" y=\"0\"/><dimensions width=\"100\" height=\"1\"/>"
            << "<material value=\"water\"/><points num_points=\"2\" x_0=\"" << x1 << "\" y_0=\"" << y1
            << "\" x_1=\"" << x2 << "\" y_1=\"" << y2 << "\"/></SolidLineStrip></level>";
        out.close();
        game::LevelScene scene;
        std::string error;
        assert(scene.Load(path.string(), renderer, error));
        assert(scene.Ground().size() == 1);
        return scene;
    };
    const auto rayOnly = sceneWithLine(-20.f, 0.f, -20.f, 60.f);
    combat.SpawnIntroBloodFlight(0.f, 0.f);
    game::CombatBloodFlightProbe::SetRandomState(combat, 0u);
    combat.UpdateBloodFlights(rayOnly, 1.f/60.f);
    const auto& directed = game::CombatBloodFlightProbe::Blood(combat);
    // Old center (0,0), advanced center (-5.6,+4.4), ray end (-56.8,+49.2).
    // The x=-20 line is crossed only by the extension; it cannot be a center hit.
    assert(directed.impacted && !directed.blocked);
    assert(Near(directed.x, -5.6f));
    const std::uint32_t jitterState = 1013904223u;
    const float jitter = static_cast<float>(jitterState % 100u) * .2f;
    const std::uint32_t signState = jitterState * 1664525u + 1013904223u;
    assert(Near(directed.y, 4.4f + ((signState & 1u) ? -jitter : jitter) - 20.f));
    assert(combat.MaterialSprayCount() == 1);
    const auto* spray = combat.MaterialSpray(0);
    assert(spray && Near(spray->x, -56.8f) && Near(spray->y, 49.2f));
    combat.ClearLevel();

    // Ordinary hit keeps its zero-direction sweep, with the ordinary new-center spray origin; it must not acquire
    // the directed 32-unit ray extension.
    const auto ordinaryLine = sceneWithLine(-50.f, 0.f, 50.f, 0.f);
    game::CombatBloodFlightProbe::SpawnOrdinary(combat, 0.f, 10.f, -20.f);
    game::CombatBloodFlightProbe::SetRandomState(combat, 0u);
    combat.UpdateBloodFlights(ordinaryLine, 1.f/60.f);
    const auto& ordinary = game::CombatBloodFlightProbe::Blood(combat);
    assert(ordinary.impacted && !ordinary.blocked && Near(ordinary.x, 0.f));
    assert(Near(ordinary.y, -20.f + ((signState & 1u) ? -jitter : jitter)));
    assert(combat.MaterialSprayCount() == 1);
    assert(Near(combat.MaterialSpray(0)->x, 0.f));
    assert(Near(combat.MaterialSpray(0)->y, -10.5f));
    std::filesystem::remove(path);
    std::cout << "PASS: directed ray-only impact center/spray and ordinary zero-direction guard\n";
}
