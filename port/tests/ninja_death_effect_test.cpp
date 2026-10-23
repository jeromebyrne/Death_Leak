#include "../game/Combat.h"
#include <cassert>
#include <cmath>
#include <string>
#include <vector>
#include <iostream>

class CaptureRenderer final : public deathleak::render::Renderer {
public:
    std::vector<std::string> paths;
    std::vector<deathleak::render::Sprite> sprites;
    bool Initialize(const char*, int, int, std::string&) override { return true; }
    deathleak::render::TextureId LoadTexture(const std::string& path, std::string&) override {
        paths.push_back(path); return static_cast<deathleak::render::TextureId>(paths.size());
    }
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
    assert(scene.Load("port/tests/fixtures/encounter_open.xml", renderer, error));
    Combat combat;
    combat.Update(scene, 0, 107.44937f, 0, false, false, true, false, 1, renderer);
    combat.SetArt(renderer);
    assert(combat.EnemyCount() == 1);
    assert(combat.DamageEnemy(0, 1));
    assert(combat.Kills() == 0 && combat.DeathDebrisCount() == 0 && combat.DeathSprayCount() == 0);
    assert(combat.DamageEnemy(0, 100));
    assert(combat.Kills() == 1 && combat.DeathDebrisCount() == 10 && combat.DeathSprayCount() == 5);
    assert(!combat.DamageEnemy(0, 100));
    assert(combat.DeathDebrisCount() == 10);
    combat.Update(scene, 100000, 100000, 0, false, false, true, false, 1, renderer);
    assert(combat.EnemyCount() == 0);
    renderer.sprites.clear();
    combat.Draw(renderer, 0, 107.44937f);
    assert(renderer.sprites.size() == 15);
    assert(renderer.paths[renderer.sprites.front().texture-1].find("ricehat.png") != std::string::npos);
    assert(std::abs(renderer.sprites.front().destination.width - 74.f*2.f/3.f) < .001f);
    const float firstX = renderer.sprites.front().destination.x;
    combat.Update(scene, 100000, 100000, 1.f/60.f, false, false, true, false, 1, renderer);
    renderer.sprites.clear(); combat.Draw(renderer, 0, 107.44937f);
    assert(renderer.sprites.size() == 15 && renderer.sprites.front().destination.x != firstX);
    combat.Update(scene, 100000, 100000, 5.f, false, false, true, false, 1, renderer);
    assert(combat.DeathDebrisCount() == 0 && combat.DeathSprayCount() == 0);
    combat.ClearLevel();
    std::cout << "PASS: ninja kill spawns once, no-kill stays empty, effects draw/move/expire\n";
}
