// Host-only CPU discriminator for controller strafe input, authored body and clock.
#include "../game/LevelScene.h"
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace deathleak;
class StrafeSpy final : public render::Renderer {
public:
    std::unordered_map<std::string, render::TextureId> textures;
    std::unordered_map<render::TextureId, std::string> paths;
    std::vector<render::Sprite> drawn;
    bool Initialize(const char*, int, int, std::string&) override { return true; }
    render::TextureId LoadTexture(const std::string& path, std::string&) override {
        auto [it, added] = textures.try_emplace(path, static_cast<render::TextureId>(textures.size()+1));
        if (added) paths.emplace(it->second, path);
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
    using game::ResolvePlayerStrafeVisual;
    using game::PlayerStrafeVisual;
    auto expect = [](render::ControllerSticks sticks, int movement, bool active, int aim = 1,
                     bool roll = false, bool melee = false, bool landed = false) {
        const auto visual = ResolvePlayerStrafeVisual(sticks, movement, roll, melee, landed);
        assert(visual.active == active && visual.movementDirection == movement);
        if (active) assert(visual.aimFacing == aim);
    };
    const render::ControllerSticks right{-9000, 9000, 0, true}, left{9000, -9000, 0, true};
    expect(right, -1, true, 1); expect(left, 1, true, -1);
    expect(right, 1, true, 1); expect(left, -1, true, -1);
    for (const auto& row : std::vector<render::ControllerSticks>{
            {}, {9000,0,0,true}, {-9000,9000,0,false},
            {9000,9000,0,true}, {-9000,-9000,0,true},
            {8689,-9000,0,true}, {-8689,9000,0,true},
            {8690,-8689,0,true}, {-8690,8689,0,true},
            {8690,-8689,8689,true}}) {
        expect(right, -1, true, 1); expect(row, -1, false); // No sticky eligibility.
    }
    expect({9000,0,8690,true}, 1, true, -1);
    expect({8690,-8689,8690,true}, 1, true, -1);
    expect({-8690,1,-8690,true}, -1, true, 1);
    expect(right, -1, false, 1, true);
    expect(right, -1, false, 1, false, true);
    expect(right, -1, false, 1, false, false, true); // Landing edge only.
    expect(right, -1, true, 1); // Next grounded tick is eligible.

    setenv("DEATHLEAK_RENDER_ONLY_XML_IDS", "2,__player_aim_line", 1);
    StrafeSpy spy;
    game::LevelScene scene;
    std::string error;
    assert(scene.Load("Game/XmlFiles/levels/forest_exploration_1.xml", spy, error));
    const float x = scene.PlayerX(), y = scene.PlayerY();
    const std::string prefix = "Game/Media/characters/player/strafe_default/strafe_default_";
    auto frame = [&](int n) {
        const std::string suffix = (n < 10 ? "00" : "0") + std::to_string(n) + ".png";
        return spy.textures.at(prefix + suffix);
    };
    for (int n = 0; n < 20; ++n) assert(frame(n) != render::InvalidTexture);
    const auto aimId = spy.textures.at("Game/Media/aim_line.png");
    // The filter permits Player ID2 and its aim-line child only. Classify every
    // draw by path so an arm or unrelated texture cannot masquerade as a body.
    auto draw = [&]() {
        spy.drawn.clear(); scene.Draw(spy, x, y, x, y);
        int bodyIndex = -1, armIndex = -1, aimIndex = -1;
        for (size_t i = 0; i < spy.drawn.size(); ++i) {
            const auto& path = spy.paths.at(spy.drawn[i].texture);
            if (path == "Game/Media/aim_line.png") {
                assert(aimIndex < 0); aimIndex = static_cast<int>(i);
            } else if (path.find("Game/Media/characters/player/back_arm_") == 0) {
                assert(armIndex < 0); armIndex = static_cast<int>(i);
            } else {
                assert(path.find("Game/Media/characters/player/") == 0);
                assert(bodyIndex < 0); bodyIndex = static_cast<int>(i);
            }
        }
        assert(bodyIndex >= 0);
        assert(armIndex < 0 || armIndex + 1 == bodyIndex);
        assert(aimIndex < 0 || aimIndex > bodyIndex);
        if (armIndex >= 0) {
            const auto& arm = spy.drawn[armIndex];
            const auto& body = spy.drawn[bodyIndex];
            assert(arm.texture != body.texture);
            assert(arm.destination.x == body.destination.x && arm.destination.y == body.destination.y);
            assert(arm.destination.width == body.destination.width &&
                   arm.destination.height == body.destination.height);
        }
        return spy.drawn[bodyIndex];
    };
    auto body = [&]() { return draw(); };
    auto update = [&](float dt, float vx, PlayerStrafeVisual visual = {}, int facing = 1,
                      bool grounded = true, int wall = 0, bool roll = false,
                      bool water = false, float melee = 0.f, bool dash = false,
                      float vy = 0.f, bool fireAttempt = false) {
        scene.UpdatePlayerVisual(dt, vx, vy, grounded, wall, roll, dash, water,
                                 melee, facing, false, fireAttempt, visual);
    };
    const PlayerStrafeVisual rightBack{true,1,-1}, leftBack{true,-1,1};
    const PlayerStrafeVisual rightForward{true,1,1}, leftForward{true,-1,-1};
    assert(draw().texture != render::InvalidTexture);
    assert(spy.drawn.size() == 2); // Initial Still arm immediately precedes body.
    // 30 FPS XML entry is used before the 47.7 FPS Player dry rate is stored.
    update(.030f, -600.f, rightBack, -1);
    assert(body().texture == frame(19) && !body().flipX);
    assert(spy.drawn.size() == 1); // No authored StrafeBack arm sequence.
    update(.001f, -600.f, rightBack, -1);
    assert(body().texture == frame(18)); // .031 > 1/47.7; stored rate applies now.
    update(.001f, -600.f, rightBack, -1);
    assert(body().texture == frame(18)); // Residual ~.01004 plus .001 is below period.
    update(.001f, 600.f, leftBack, 1); // Same sequence: no restart.
    assert(body().texture == frame(18) && body().flipX);
    update(.001f, 600.f, rightForward, 1);
    assert(body().texture == frame(0) && !body().flipX);
    update(.034f, 600.f, rightForward, 1);
    assert(body().texture == frame(1));
    update(.001f, -600.f, leftForward, -1); // Same forward sequence, changed body flip.
    assert(body().texture == frame(1) && body().flipX);
    update(.001f, 600.f, {});
    assert(spy.paths.at(body().texture).find("/run_") != std::string::npos);
    update(.021f, -600.f, rightBack, -1); // Rate persisted at 47.7, clock restarted.
    assert(body().texture == frame(18));
    update(.001f, 0.f, rightBack);
    assert(spy.paths.at(body().texture).find("/idle_") != std::string::npos);
    update(.001f, -600.f, rightBack, -1, true, 1);
    assert(spy.paths.at(body().texture).find("/run_") != std::string::npos);
    update(.001f, -600.f, rightBack, -1, true, 0, true);
    assert(spy.paths.at(body().texture).find("/roll_") != std::string::npos);
    update(.001f, -600.f, rightBack, -1, true, 0, false, true);
    assert(spy.paths.at(body().texture).find("/swim_") != std::string::npos);
    update(.001f, -600.f, rightBack, -1, true, 0, false, false, .01f);
    assert(spy.paths.at(body().texture).find("/melee_") != std::string::npos);
    update(.001f, -600.f, rightBack, -1, true, 0, false, false, 0.f, true);
    assert(spy.paths.at(body().texture).find("/jump_") != std::string::npos);
    update(.001f, -600.f, rightBack, -1, false, 0, false, false, 0.f, false, 100.f);
    assert(spy.paths.at(body().texture).find("/jump_") != std::string::npos);
    update(.001f, -600.f, rightBack, -1, false, 1, false, false, 0.f, false, -100.f);
    assert(spy.paths.at(body().texture).find("/wall_slide_") != std::string::npos);
    update(.001f, 3.f, rightBack, -1);
    assert(spy.paths.at(body().texture).find("/idle_") != std::string::npos);

    // Strafe footstep-marked frames never dispatch Running's material spray.
    scene.SetPlayerFootstepContact(0, x, y);
    const size_t sprays = scene.FootstepSprays().size();
    for (int i = 0; i < 50; ++i) update(.05f, -600.f, rightBack, -1);
    assert(scene.FootstepSprays().size() == sprays);

    // Positive control on a fresh scene: the same grass contact must produce
    // Running's authored frame-0 footstep. This makes the strafe zero meaningful.
    StrafeSpy spraySpy;
    game::LevelScene sprayScene;
    assert(sprayScene.Load("Game/XmlFiles/levels/forest_exploration_1.xml", spraySpy, error));
    assert(spraySpy.textures.count("Game/Media/rock_particle.png") == 1);
    sprayScene.SetPlayerFootstepContact(0, sprayScene.PlayerX(), sprayScene.PlayerY());
    assert(sprayScene.FootstepSprays().empty());
    sprayScene.UpdatePlayerVisual(.001f, 600.f, 0.f, true, 0, false, false,
                                  false, 0.f, 1, false, false, {});
    assert(sprayScene.FootstepSprays().size() == 1);
    const auto& runningSpray = sprayScene.FootstepSprays().front();
    assert(runningSpray.texture == spraySpy.textures.at("Game/Media/rock_particle.png"));
    assert(runningSpray.particles.size() == 3);
    const size_t positiveCount = sprayScene.FootstepSprays().size();
    for (int i = 0; i < 50; ++i)
        sprayScene.UpdatePlayerVisual(.05f, -600.f, 0.f, true, 0, false, false,
                                      false, 0.f, -1, false, false, rightBack);
    assert(sprayScene.FootstepSprays().size() == positiveCount);

    update(.001f, 0.f, {}, -1, true, 0, false, false, 0.f, false, 0.f, true);
    draw();
    render::Sprite baselineAim{}; bool sawAim = false;
    for (const auto& sprite : spy.drawn) if (sprite.texture == aimId) { baselineAim = sprite; sawAim = true; }
    assert(sawAim);
    update(.001f, -600.f, rightBack, -1, true, 0, false, false, 0.f, false, 0.f, true);
    const auto strafeBody = body();
    assert(strafeBody.texture == frame(19) && !strafeBody.flipX);
    sawAim = false;
    for (const auto& sprite : spy.drawn) if (sprite.texture == aimId) {
        assert(sprite.destination.x == baselineAim.destination.x);
        assert(sprite.destination.y == baselineAim.destination.y);
        assert(sprite.destination.width == baselineAim.destination.width);
        assert(sprite.destination.height == baselineAim.destination.height);
        assert(sprite.flipX == baselineAim.flipX && sprite.flipY == baselineAim.flipY);
        sawAim = true;
    }
    assert(sawAim);
    assert(spy.drawn.size() == 2); // Body, then combat-facing aim line; no strafe arm.
    // Observe every authored body frame in XML order, independent of cached IDs.
    update(.001f, 600.f, rightForward, 1);
    assert(body().texture == frame(0));
    for (int n = 1; n < 20; ++n) {
        update(.1f, 600.f, rightForward, 1);
        assert(body().texture == frame(n)); // One step max, even with excess time.
    }
    update(.001f, -600.f, rightBack, -1);
    assert(body().texture == frame(19));
    for (int n = 18; n >= 0; --n) {
        update(.1f, -600.f, rightBack, -1);
        assert(body().texture == frame(n));
    }
    // Fresh low-speed sequence: QA's old .010 + .024 expectation was early.
    StrafeSpy slowSpy;
    game::LevelScene slowScene;
    assert(slowScene.Load("Game/XmlFiles/levels/forest_exploration_1.xml", slowSpy, error));
    auto slowBody = [&]() {
        slowSpy.drawn.clear();
        slowScene.Draw(slowSpy, slowScene.PlayerX(), slowScene.PlayerY(),
                       slowScene.PlayerX(), slowScene.PlayerY());
        assert(slowSpy.drawn.size() == 1);
        return slowSpy.paths.at(slowSpy.drawn[0].texture);
    };
    auto slowUpdate = [&](float dt) {
        slowScene.UpdatePlayerVisual(dt, -100.f, 0.f, true, 0, false, false,
                                     false, 0.f, -1, false, false, rightBack);
    };
    slowUpdate(.010f);
    assert(slowBody() == prefix + "019.png");
    slowUpdate(.024f); // Stored rate is 7.95 FPS; .034 < 1/7.95.
    assert(slowBody() == prefix + "019.png");
    slowUpdate(.092f); // .126 > 1/7.95, so exactly one frame advances.
    assert(slowBody() == prefix + "018.png");
    std::cout << "PASS: strafe raw-input gate, body selection/flip, clock, priorities, spray, aim-line\n";
}
