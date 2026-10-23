// Isolated CPU discriminator for source Roll footfalls and the unstamped running-frame guard.
#include "../game/LevelScene.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <string>
#include <unordered_map>

class CaptureRenderer final : public deathleak::render::Renderer {
public:
    std::unordered_map<std::string, deathleak::render::TextureId> textures;
    bool Initialize(const char*, int, int, std::string&) override { return true; }
    deathleak::render::TextureId LoadTexture(const std::string& path, std::string&) override {
        return textures.try_emplace(path, static_cast<deathleak::render::TextureId>(textures.size() + 1)).first->second;
    }
    void BeginFrame(deathleak::render::Color) override {}
    void DrawSprite(const deathleak::render::Sprite&) override {}
    void FillRect(deathleak::render::Rect, deathleak::render::Color) override {}
    void ApplyScenePostprocess() override {}
    void EndFrame() override {}
    bool PollQuit() override { return false; }
    bool KeyDown(int) const override { return false; }
    deathleak::render::MouseState Mouse() const override { return {}; }
    bool SaveFrame(const std::string&, std::string&) override { return false; }
};

int main() {
    CaptureRenderer renderer;
    deathleak::game::LevelScene scene;
    std::string error;
    assert(scene.Load("Game/XmlFiles/levels/forest_exploration_1.xml", renderer, error));
    const auto& line = scene.Ground().front();
    assert(line.material == "grassground");
    const float x = (line.x1 + line.x2) * .5f;
    const float y = line.y1 + (line.y2-line.y1)*.5f + 214.898743f*.5f;
    const auto setContact = [&](size_t lineIndex) { scene.SetPlayerFootstepContact(lineIndex, x, y); };
    const auto tick = [&](float dt, bool rolling, bool grounded = true, int direction = -1) {
        scene.UpdatePlayerVisual(dt, rolling ? 0.f : 480.f, 0, grounded, 0, rolling,
                                 false, false, 0, 1, false, false, {false, 1, direction});
    };
    constexpr float frame = 1.f / 60.f;
    setContact(0);
    for (int i = 0; i < 9; ++i) tick(frame, true);
    assert(scene.FootstepSprays().empty()); // Unmarked frames 1..9.
    tick(frame, true);
    assert(scene.FootstepSprays().size() == 1);
    const auto& first = scene.FootstepSprays().front();
    assert(first.texture == renderer.textures.at("Game/Media/rock_particle.png"));
    assert(first.particles.size() == 3 && first.depth == 3000);
    assert(std::abs(first.x - (x - 12.f)) < .01f);
    assert(std::abs(first.y - (y - 214.898743f*.5f + 17.f)) < .01f);
    for (const auto& particle : first.particles)
        assert(particle.directionX >= .5f && particle.directionX <= .59f);
    tick(frame / 4.f, true);
    assert(scene.FootstepSprays().size() == 2); // Source repeats on held marked Roll frame.
    for (int i = 0; i < 6; ++i) tick(frame, true);
    assert(scene.FootstepSprays().size() == 3); // Marked frame 16.
    tick(frame / 4.f, true);
    assert(scene.FootstepSprays().size() == 4);

    deathleak::game::LevelScene negative;
    assert(negative.Load("Game/XmlFiles/levels/forest_exploration_1.xml", renderer, error));
    negative.SetPlayerFootstepContact(0, x, y);
    // Running frame 10 stamps the shared numeric guard.
    for (int i = 0; i < 40 && negative.FootstepSprays().size() < 2; ++i)
        negative.UpdatePlayerVisual(frame, 480, 0, true, 0, false, false, false, 0, 1, false);
    assert(negative.FootstepSprays().size() == 2);
    for (int i = 0; i < 10; ++i)
        negative.UpdatePlayerVisual(frame, 0, 0, true, 0, true, false, false, 0, 1, false);
    assert(negative.FootstepSprays().size() == 2); // Roll frame 10 equals stored running frame.
    for (int i = 0; i < 6; ++i)
        negative.UpdatePlayerVisual(frame, 0, 0, true, 0, true, false, false, 0, 1, false);
    assert(negative.FootstepSprays().size() == 3); // Roll frame 16 differs.

    deathleak::game::LevelScene airborne;
    assert(airborne.Load("Game/XmlFiles/levels/forest_exploration_1.xml", renderer, error));
    airborne.SetPlayerFootstepContact(static_cast<size_t>(-1), x, y);
    for (int i = 0; i < 17; ++i)
        airborne.UpdatePlayerVisual(frame, 0, 0, false, 0, true, false, false, 0, 1, false);
    assert(airborne.FootstepSprays().empty());
    std::cout << "PASS: Roll marked material sprays, held-frame repeat, running-frame guard, lost contact\n";
}
