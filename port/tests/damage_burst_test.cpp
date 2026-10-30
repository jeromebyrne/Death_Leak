#include "../game/Combat.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

class CaptureRenderer final : public deathleak::render::Renderer {
public:
    std::vector<deathleak::render::Sprite> sprites;
    bool Initialize(const char*, int, int, bool, std::string&) override { return true; }
    deathleak::render::TextureId LoadTexture(const std::string&, std::string&) override { return 1; }
    void BeginFrame(deathleak::render::Color) override {}
    void DrawSprite(const deathleak::render::Sprite& sprite) override { sprites.push_back(sprite); }
    void FillRect(deathleak::render::Rect, deathleak::render::Color) override {}
    void ApplyScenePostprocess() override {}
    void EndFrame() override {}
    bool PollQuit() override { return false; }
    bool KeyDown(int) const override { return false; }
    deathleak::render::MouseState Mouse() const override { return {}; }
    bool SaveFrame(const std::string&, std::string&) override { return false; }
};

int main() {
    using namespace deathleak::game;
    CaptureRenderer renderer;
    LevelScene scene;
    std::string error;
    assert(scene.Load("port/tests/fixtures/damage_burst.xml", renderer, error));
    Combat combat;
    auto drawPlayer = [&](float expected) {
        renderer.sprites.clear();
        scene.Draw(renderer, 0, 0, 0, 0, {}, combat.PlayerDamageBurstGain());
        int boosted = 0;
        for (const auto& sprite : renderer.sprites) {
            // kPlayer's 110-world-unit fixture body, not adjacent background/foreground.
            if (std::abs(sprite.destination.width - 110.f*2.f/3.f) < .01f) {
                assert(std::abs(sprite.rgbGain - expected) < .0001f);
                assert(sprite.tint.a == 255 && sprite.opacity == 1.f);
                ++boosted;
            } else assert(sprite.rgbGain == 1.f);
        }
        assert(boosted == 1);
    };
    drawPlayer(1.f);
    combat.ApplySolidContactDamage(2.f);
    drawPlayer(5.f);
    assert(combat.Health() == 23.f);
    combat.Update(scene, 100000, 100000, .125f, false, false, true, false, 1, renderer);
    drawPlayer(3.f);
    combat.ApplySolidContactDamage(1.f); // A second accepted hit restarts this actor only.
    drawPlayer(5.f);
    combat.Update(scene, 100000, 100000, .25f, false, false, true, false, 1, renderer);
    drawPlayer(1.f);
    combat.ClearLevel();
    drawPlayer(1.f);
    combat.ApplySolidContactDamage(0.f);
    drawPlayer(1.f);

    LevelScene encounter;
    assert(encounter.Load("port/tests/fixtures/encounter_open.xml", renderer, error));
    Combat enemyCombat;
    enemyCombat.Update(encounter, 0, 107.44937f, 0, false, false, true, false, 1, renderer);
    assert(enemyCombat.EnemyCount() == 1);
    auto enemyGain = [&]() {
        renderer.sprites.clear();
        enemyCombat.Draw(renderer, 0, 107.44937f);
        assert(renderer.sprites.size() == 1);
        assert(renderer.sprites[0].tint.a == 255);
        return renderer.sprites[0].rgbGain;
    };
    assert(enemyGain() == 1.f);
    assert(enemyCombat.DamageEnemy(0, 1.f));
    assert(enemyGain() == 5.f);
    enemyCombat.Update(encounter, 100000, 100000, .125f, false, false, true, false, 1, renderer);
    assert(enemyGain() == 3.f);
    assert(enemyCombat.DamageEnemy(0, 1.f));
    assert(enemyGain() == 5.f);
    enemyCombat.Update(encounter, 100000, 100000, .25f, false, false, true, false, 1, renderer);
    assert(enemyGain() == 1.f);
    std::cout << "PASS: independent actor RGB burst, hit reset, endpoint and unchanged alpha/neighbors\n";
}
