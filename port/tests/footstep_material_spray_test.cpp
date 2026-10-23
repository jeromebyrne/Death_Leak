// CPU discriminator for source-authored Running footfall frames and grounded materials.
#include "../game/LevelScene.h"
#include "../game/PlayerMotion.h"
#include <cassert>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iterator>
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
    const auto& ground = scene.Ground();
    assert(ground.size() >= 2 && ground[0].material == "grassground" && ground[1].material == "softwood");
    constexpr float dt = 1.f / 60.f;
    const float x = (ground[0].x1 + ground[0].x2) * .5f;
    const float y = ground[0].y1 + (ground[0].y2-ground[0].y1)*.5f + 214.898743f*.5f;
    scene.SetPlayerFootstepContact(0, x, y);
    auto step = [&](float vx, bool grounded = true, bool water = false, int facing = 1) {
        scene.UpdatePlayerVisual(dt, vx, 0, grounded, 0, false, false, water, 0, facing, false);
    };
    step(480);
    assert(scene.FootstepSprays().size() == 1); // Entry into Running frame 0.
    const auto& first = scene.FootstepSprays().front();
    assert(first.particles.size() == 3 && first.depth == 3000);
    assert(first.texture == renderer.textures.at("Game/Media/rock_particle.png"));
    assert(std::abs(first.x-(x+12.f)) < .01f && std::abs(first.y-(y-214.898743f*.5f+17.f)) < .01f);
    assert(first.fadeIn == .15f && first.fadeOut == .7f && first.scaleTo == 2.5f);
    for (const auto& p : first.particles) {
        assert(p.startX == first.x && p.startY == first.y && p.gravity == .5f);
        assert(p.directionX >= -.5f && p.directionX <= -.41f && p.directionY >= .41f && p.directionY <= .5f);
        assert(p.speed >= 2.f && p.speed <= 4.f && p.size >= 40.f && p.size <= 50.f);
        assert(p.brightness >= .8f && p.brightness <= 1.f);
    }
    step(480);
    assert(scene.FootstepSprays().size() == 1); // Same frame: no repeat.
    int ticksToSecond = 0;
    while (ticksToSecond < 40 && scene.FootstepSprays().size() == 1) {
        step(480);
        ++ticksToSecond;
    }
    assert(ticksToSecond >= 10 && scene.FootstepSprays().size() == 2); // Running frame 10 only.
    step(480);
    assert(scene.FootstepSprays().size() == 2); // No repeated tick on frame 10.
    // Source keeps mLastRunFramePlayed across Still. Re-entering the same
    // numbered marked frame must not replay it; a different frame must.
    step(0);
    scene.SetPlayerFootstepContact(0, x, y);
    step(480); // frame 0 differs from prior Running frame 10.
    assert(scene.FootstepSprays().size() == 3);
    step(0);
    step(480); // frame 0 again, even though this is a new sequence entry.
    assert(scene.FootstepSprays().size() == 3);
    step(0);
    scene.SetPlayerFootstepContact(static_cast<size_t>(-1), x, y);
    step(480, false);
    assert(scene.FootstepSprays().size() == 3);
    step(0, true);
    scene.SetPlayerFootstepContact(ground.size()-1, x, y); // No particle texture.
    step(480); // Last Running frame was 0: same-frame gate still holds.
    assert(scene.FootstepSprays().size() == 3);
    // The physical contact path reports the actual winning line index.
    deathleak::game::PlayerMotion motion;
    motion.Reset(x, y, true);
    motion.Update(scene, dt, 1, false);
    assert(motion.Grounded());
    for (int ticks = 0; ticks < 40 && scene.FootstepSprays().size() == 3; ++ticks) step(480);
    assert(scene.FootstepSprays().size() == 4);
    assert(std::abs(scene.FootstepSprays().back().x - (motion.X()+12.f)) < .01f);
    step(0);
    scene.SetPlayerFootstepContact(0, x, y);
    step(-480, true, false, -1);
    assert(scene.FootstepSprays().size() == 5); // frame 0 after frame 10.
    assert(std::abs(scene.FootstepSprays().back().x-(x-12.f)) < .01f);
    assert(scene.FootstepSprays().back().particles.front().directionX >= .5f);

    // Make a private temporary level by changing only the first segment's
    // material. This exercises XML material loading, not a test-only hook.
    const auto source = std::filesystem::path("Game/XmlFiles/levels/forest_exploration_1.xml");
    std::ifstream input(source);
    assert(input);
    const std::string original((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    const auto materialAt = original.find("<material value=\"grassground\" />");
    assert(materialAt != std::string::npos);
    auto checkMaterial = [&](const std::string& material, bool expectSpray) {
        auto xml = original;
        xml.replace(materialAt, std::string("<material value=\"grassground\" />").size(),
                    "<material value=\"" + material + "\" />");
        const auto fixture = std::filesystem::temp_directory_path() /
            ("deathleak_footstep_" + material + "_" + std::to_string(std::hash<std::string>{}(source.string())) + ".xml");
        { std::ofstream out(fixture); assert(out); out << xml; }
        deathleak::game::LevelScene materialScene;
        std::string loadError;
        const bool loaded = materialScene.Load(fixture.string(), renderer, loadError);
        std::filesystem::remove(fixture);
        assert(loaded && materialScene.Ground().front().material == material);
        materialScene.SetPlayerFootstepContact(0, x, y);
        materialScene.UpdatePlayerVisual(dt, 480, 0, true, 0, false, false, false, 0, 1, false);
        assert((materialScene.FootstepSprays().size() == 1) == expectSpray);
        if (expectSpray)
            assert(materialScene.FootstepSprays().front().texture ==
                   renderer.textures.at("Game/Media/large_splash.png"));
    };
    checkMaterial("shallow_water", true); // IsWater() is false for this exact name.
    checkMaterial("water", false);         // Only exact water suppresses particles.
    std::cout << "PASS: material Running footstep frame gate, contact, exact water predicate, seeded spray\n";
}
