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
    assert(combat.MaterialSprayCount() == 0);
    const float originX = combat.Enemies()[0].x;
    const float originY = combat.Enemies()[0].y;
    assert(combat.DamageEnemy(0, 100, 1.f));
    assert(combat.Kills() == 1 && combat.DeathDebrisCount() == 10 && combat.DeathSprayCount() == 5);
    assert(combat.MaterialSprayCount() == 1);
    const auto* material = combat.MaterialSpray(0);
    assert(material && material->particles.size() == 20);
    assert(renderer.paths[material->texture-1] == "Game/Media/bloodparticle.png");
    assert(std::abs(material->x - (originX + 30.f)) < .001f);
    assert(std::abs(material->y - (originY - 231.714279f/2.f + 30.f)) < .001f);
    assert(material->depth == 3000 && !material->looping && material->warmed);
    assert(material->width == 3200.f && material->height == 2000.f);
    assert(material->fadeIn == .15f && material->fadeOut == .8f);
    assert(material->scalesByLifetime && material->scaleTo == 8.f);
    for (size_t i = 0; i < material->particles.size(); ++i) {
        const auto& p = material->particles[i];
        assert(p.directionX >= .2f && p.directionX <= .49f);
        assert(std::abs(p.directionX + p.directionY - 1.f) < .0001f);
        assert(std::abs(p.x-material->x) <= 50.f && std::abs(p.y-material->y) <= 100.f);
        assert(p.size >= 11.f && p.size <= 40.f);
        assert(p.speed >= 7.01f && p.speed <= 12.f);
        assert(p.gravity == 1.4f && p.brightness >= .75f && p.brightness <= 1.f);
        assert(std::abs(p.lifetime - (1.5f-.035f*static_cast<float>(i))) < .0001f);
    }
    assert(!combat.DamageEnemy(0, 100));
    assert(combat.DeathDebrisCount() == 10 && combat.MaterialSprayCount() == 1);
    combat.Update(scene, 100000, 100000, 0, false, false, true, false, 1, renderer);
    assert(combat.EnemyCount() == 0);
    renderer.sprites.clear();
    combat.Draw(renderer, 0, 107.44937f);
    assert(renderer.sprites.size() == 35);
    assert(renderer.paths[renderer.sprites.front().texture-1].find("ricehat.png") != std::string::npos);
    assert(std::abs(renderer.sprites.front().destination.width - 74.f*2.f/3.f) < .001f);
    const float firstX = renderer.sprites.front().destination.x;
    combat.Update(scene, 100000, 100000, 1.f/60.f, false, false, true, false, 1, renderer);
    renderer.sprites.clear(); combat.Draw(renderer, 0, 107.44937f);
    assert(renderer.sprites.size() == 35 && renderer.sprites.front().destination.x != firstX);
    combat.Update(scene, 100000, 100000, 5.f, false, false, true, false, 1, renderer);
    assert(combat.DeathDebrisCount() == 0 && combat.DeathSprayCount() == 0 && combat.MaterialSprayCount() == 0);
    combat.ClearLevel();
    std::cout << "PASS: ninja kill spawns once, no-kill stays empty, effects draw/move/expire\n";
}
