#include "../game/Combat.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

class CaptureRenderer final : public deathleak::render::Renderer {
public:
    std::vector<deathleak::render::Sprite> sprites;
    bool Initialize(const char*, int, int, std::string&) override { return true; }
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
    LevelScene empty;
    Combat combat;
    auto tick = [&](float dt, bool fire) {
        combat.Update(empty, 100000.f, 100000.f, dt, fire, false, true, false, -1, renderer);
    };
    auto expect = [&](float age, float fraction) {
        assert(combat.ShotCount() == 1);
        const auto& shot = combat.Shots().front();
        assert(shot.playerKnife && std::abs(shot.activeVisualAge - age) < .0001f);
        renderer.sprites.clear();
        combat.Draw(renderer, shot.x, shot.y);
        if (fraction == 0.f) { assert(renderer.sprites.empty()); return; }
        assert(renderer.sprites.size() == 1);
        const auto& sprite = renderer.sprites.front();
        assert(std::abs(sprite.destination.width - 84.5f * fraction * 2.f/3.f) < .001f);
        assert(std::abs(sprite.destination.height - 25.f * fraction * 2.f/3.f) < .001f);
        assert(sprite.tint.a == std::lround(255.f * fraction));
        assert(sprite.flipX);
    };
    tick(0.f, true); expect(0.f, 0.f);
    tick(.15f, false); expect(.15f, .5f);
    tick(.15f, false); expect(.30f, 1.f);
    tick(.05f, false); expect(.35f, 1.f); // Must survive beyond old 0.25 s active TTL.
    for (int i = 0; i < 9; ++i) tick(1.f, false);
    assert(combat.ShotCount() == 1);
    tick(1.f, false);
    assert(combat.ShotCount() == 0); // Port-only offscreen collection bound.
    LevelScene solid;
    std::string error;
    assert(solid.Load("port/tests/fixtures/solid_box.xml", renderer, error));
    assert(!solid.SolidBoxes().empty());
    Combat contact;
    contact.Update(solid, 0.f, 200.f, 0.f, true, false, true, false, 1, renderer);
    assert(contact.ShotCount() == 1);
    contact.Update(solid, 0.f, 200.f, .15f, false, false, true, false, 1, renderer);
    assert(contact.ShotCount() == 0); // Existing solid contact still retires immediately.
    std::cout << "PASS: player knife fade-in, active-flight bound and solid contact\n";
}
