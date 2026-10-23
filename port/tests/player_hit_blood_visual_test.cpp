#include "../game/Combat.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

using namespace deathleak;
namespace deathleak::game {
struct CombatBloodFlightProbe {
    static void Inject(game::Combat& combat, float x, float y, float damage = 2.f) {
        game::Shot shot{x, y, 0.f, 0.f, 2.f, damage, true};
        shot.gravity = false;
        shot.deflectable = false;
        combat.shots_.push_back(shot);
    }
    static void ClearShots(game::Combat& combat) { combat.shots_.clear(); }
};
} // namespace deathleak::game
class CaptureRenderer final : public render::Renderer {
public:
    render::TextureId blood = 0;
    std::vector<render::Sprite> sprites;
    bool Initialize(const char*, int, int, std::string&) override { return true; }
    render::TextureId LoadTexture(const std::string& path, std::string&) override {
        if (path == "Game/Media/blood_projectile.png") return blood = 77;
        return 1;
    }
    void BeginFrame(render::Color) override {}
    void DrawSprite(const render::Sprite& sprite) override { sprites.push_back(sprite); }
    void FillRect(render::Rect, render::Color) override {}
    void ApplyScenePostprocess() override {}
    void EndFrame() override {}
    bool PollQuit() override { return false; }
    bool KeyDown(int) const override { return false; }
    render::MouseState Mouse() const override { return {}; }
    bool SaveFrame(const std::string&, std::string&) override { return false; }
};
int main() {
    game::Combat combat;
    game::LevelScene scene;
    CaptureRenderer renderer;
    constexpr float x = 10000.f, y = 10000.f;
    auto tick = [&](float dt) {
        combat.Update(scene, x, y, dt, false, false, true, false, 1, renderer);
    };
    auto blood = [&](float fraction, size_t count, float worldY) {
        renderer.sprites.clear();
        combat.Draw(renderer, x, y);
        std::vector<render::Sprite> found;
        for (const auto& sprite : renderer.sprites) if (sprite.texture == renderer.blood) found.push_back(sprite);
        assert(found.size() == count);
        std::vector<game::PainterItem> items;
        combat.AppendPainterItems(items, renderer, x, y);
        size_t bloodItems = 0;
        for (const auto& item : items) {
            renderer.sprites.clear();
            item.draw();
            if (!renderer.sprites.empty() && renderer.sprites.back().texture == renderer.blood) {
                assert(item.depth == 3000);
                ++bloodItems;
            }
        }
        assert(bloodItems == count);
        if (!found.empty()) {
            const auto& sprite = found.front();
            const float worldSize = sprite.destination.width * 1.5f;
            assert(worldSize >= 128.f*fraction - .01f && worldSize <= 250.88f*fraction + .01f);
            assert(std::abs(sprite.destination.height - sprite.destination.width) < .001f);
            assert(sprite.tint.a == std::lround(255.f*fraction));
            assert(std::abs(sprite.destination.x + sprite.destination.width/2.f - 640.f) < .001f);
            assert(std::abs(sprite.destination.y + sprite.destination.height/2.f -
                            (360.f + (y-worldY)*2.f/3.f)) < .01f);
        }
    };
    game::CombatBloodFlightProbe::Inject(combat, x, y);
    tick(0.f);
    assert(combat.Hits() == 1 && combat.Health() == 23.f);
    assert(combat.PlayerBloodFlightCount() == 1 && renderer.blood == 77);
    blood(0.f, 1, y);
    game::CombatBloodFlightProbe::Inject(combat, x, y);
    tick(.1f); // still inside the 0.5-second hurt gate
    assert(combat.Hits() == 1 && combat.PlayerBloodFlightCount() == 1);
    blood(1.f/3.f, 1, y-2.f);
    game::CombatBloodFlightProbe::ClearShots(combat);
    tick(.2f);
    blood(1.f, 1, y-6.f);
    tick(.21f);
    game::CombatBloodFlightProbe::Inject(combat, x, y, 0.f);
    tick(0.f); // zero damage is not a real hit
    assert(combat.PlayerBloodFlightCount() == 1);
    tick(.5f); // the existing port also starts cooldown for zero-damage contact
    game::CombatBloodFlightProbe::Inject(combat, x, y);
    tick(0.f);
    assert(combat.Hits() == 3 && combat.PlayerBloodFlightCount() == 2);
    combat.ClearLevel();
    assert(combat.PlayerBloodFlightCount() == 0);
    std::cout << "PASS: one active blood flight per accepted player hit, fade, hurt gate, rejected damage and clear\n";
}
