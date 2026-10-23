// CPU-only authored strafe material-footfall discriminator.
#include "../game/LevelScene.h"
#include <cassert>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <unordered_map>
#include <vector>

using namespace deathleak;
class Spy final : public render::Renderer {
public:
    std::unordered_map<std::string, render::TextureId> textures;
    std::unordered_map<render::TextureId, std::string> paths;
    std::vector<render::Sprite> drawn;
    bool Initialize(const char*, int, int, std::string&) override { return true; }
    render::TextureId LoadTexture(const std::string& path, std::string&) override {
        auto [it, inserted] = textures.try_emplace(path, static_cast<render::TextureId>(textures.size()+1));
        if (inserted) paths.emplace(it->second, path);
        return it->second;
    }
    void BeginFrame(render::Color) override {}
    void DrawSprite(const render::Sprite& sprite) override { drawn.push_back(sprite); }
    void FillRect(render::Rect, render::Color) override {}
    void ApplyScenePostprocess() override {}
    void EndFrame() override {}
    bool PollQuit() override { return false; }
    bool KeyDown(int) const override { return false; }
    render::MouseState Mouse() const override { return {}; }
    bool SaveFrame(const std::string&, std::string&) override { return false; }
};

int main() {
    Spy spy;
    game::LevelScene scene;
    std::string error;
    assert(scene.Load("Game/XmlFiles/levels/forest_exploration_1.xml", spy, error));
    const auto& ground = scene.Ground();
    assert(!ground.empty() && ground[0].material == "grassground");
    const float x = scene.PlayerX(), y = scene.PlayerY();
    const auto rock = spy.textures.at("Game/Media/rock_particle.png");
    const game::PlayerStrafeVisual forward{true, 1, 1};
    const game::PlayerStrafeVisual back{true, 1, -1};
    auto tick = [&](float dt, float vx, game::PlayerStrafeVisual visual, bool grounded = true,
                    bool water = false, int contact = 0) {
        scene.SetPlayerFootstepContact(contact < 0 ? static_cast<size_t>(-1) : static_cast<size_t>(contact), x, y);
        scene.UpdatePlayerVisual(dt, vx, 0.f, grounded, 0, false, false, water,
                                 0.f, visual.active ? visual.aimFacing : (vx < 0 ? -1 : 1),
                                 false, false, visual);
    };
    auto check = [&](size_t count, float origin, int direction) {
        assert(scene.FootstepSprays().size() == count);
        const auto& spray = scene.FootstepSprays().back();
        assert(spray.texture == rock && spray.particles.size() == 3 && spray.depth == 3000);
        assert(std::abs(spray.x - origin) < .01f);
        assert(std::abs(spray.y - (y - 214.898743f*.5f + 17.f)) < .01f);
        assert(spray.fadeIn == .15f && spray.fadeOut == .7f && spray.scaleTo == 2.5f);
        for (const auto& p : spray.particles) {
            assert(p.startX == spray.x && p.startY == spray.y);
            assert(p.directionX * direction < 0.f && p.directionY > 0.f);
            assert(p.speed >= 2.f && p.speed <= 4.f && p.size >= 40.f && p.size <= 50.f);
        }
    };
    tick(.001f, 600.f, forward); // Forward marked frame 0.
    check(1, x+12.f, 1);
    tick(.001f, 600.f, forward);
    assert(scene.FootstepSprays().size() == 1);
    for (int frame = 1; frame <= 10; ++frame) {
        tick(.1f, 600.f, forward); // At most one frame per update.
        assert(scene.FootstepSprays().size() == (frame == 10 ? 2u : 1u));
    }
    check(2, x+12.f, 1);
    tick(.001f, -600.f, back); // Back entry index 0 is unmarked (texture 019).
    assert(scene.FootstepSprays().size() == 2);
    for (int frame = 1; frame <= 9; ++frame) {
        tick(.1f, -600.f, back);
        assert(scene.FootstepSprays().size() == (frame == 9 ? 3u : 2u));
    }
    check(3, x-12.f, -1); // Opposed aim faces right; spray follows left movement.
    spy.drawn.clear();
    scene.Draw(spy, x, y, x, y);
    bool sawBody = false;
    for (const auto& sprite : spy.drawn) {
        const auto& path = spy.paths.at(sprite.texture);
        if (path.find("Game/Media/characters/player/strafe_default/") == 0) {
            assert(!sprite.flipX); // Rightward aim/body despite leftward foot spray.
            sawBody = true;
        }
    }
    assert(sawBody);
    for (int frame = 10; frame <= 19; ++frame) {
        tick(.1f, -600.f, back);
        assert(scene.FootstepSprays().size() == (frame == 19 ? 4u : 3u));
    }
    check(4, x-12.f, -1);
    tick(.001f, 0.f, {}); // Still retains shared last numeric frame 19.
    tick(.001f, -600.f, back); // Back entry index 0 is unmarked.
    assert(scene.FootstepSprays().size() == 4);
    tick(.001f, 600.f, forward); // Entry 0 equals shared back index 0.
    assert(scene.FootstepSprays().size() == 4);
    tick(.001f, 0.f, {});
    tick(.001f, 600.f, forward); // Same index 0 across sequence switch: no replay.
    assert(scene.FootstepSprays().size() == 4);
    tick(.001f, 0.f, {});
    tick(.001f, -600.f, back, false); // Airborne cannot emit.
    assert(scene.FootstepSprays().size() == 4);
    tick(.001f, 0.f, {});
    tick(.001f, 600.f, forward, true, false, -1); // No line contact.
    assert(scene.FootstepSprays().size() == 4);
    tick(.001f, 0.f, {});
    tick(.001f, -600.f, back, true, true); // Water animation preempts strafe.
    assert(scene.FootstepSprays().size() == 4);

    // Exact water material suppresses particles even with dry strafe animation.
    const auto source = std::filesystem::path("Game/XmlFiles/levels/forest_exploration_1.xml");
    std::ifstream input(source); assert(input);
    std::string xml((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    const auto pos = xml.find("<material value=\"grassground\" />");
    assert(pos != std::string::npos);
    xml.replace(pos, std::string("<material value=\"grassground\" />").size(),
                "<material value=\"water\" />");
    const auto temp = std::filesystem::temp_directory_path() / "deathleak_strafe_water_fixture.xml";
    { std::ofstream output(temp); assert(output); output << xml; }
    game::LevelScene wet;
    const bool loaded = wet.Load(temp.string(), spy, error);
    std::filesystem::remove(temp);
    assert(loaded && wet.Ground()[0].material == "water");
    wet.SetPlayerFootstepContact(0, x, y);
    wet.UpdatePlayerVisual(.001f, -600.f, 0.f, true, 0, false, false, false,
                           0.f, 1, false, false, back);
    assert(wet.FootstepSprays().empty());

    // Fresh Running positive control retains the original frame-0 burst.
    game::LevelScene running;
    assert(running.Load("Game/XmlFiles/levels/forest_exploration_1.xml", spy, error));
    running.SetPlayerFootstepContact(0, x, y);
    running.UpdatePlayerVisual(.001f, 600.f, 0.f, true, 0, false, false, false,
                               0.f, 1, false, false, {});
    assert(running.FootstepSprays().size() == 1);
    assert(running.FootstepSprays()[0].texture == rock);
    std::cout << "PASS: authored strafe footstep frames, shared guard, movement direction, contact/water, Running\n";
}
