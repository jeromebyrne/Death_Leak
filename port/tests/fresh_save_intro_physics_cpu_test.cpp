// Focused CPU trace for the fresh-save unpulled-sword motion seam.
#include "../game/LevelScene.h"
#include "../game/PlayerMotion.h"
#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

class SpyRenderer final : public deathleak::render::Renderer {
public:
    bool Initialize(const char*, int, int, std::string&) override { return true; }
    deathleak::render::TextureId LoadTexture(const std::string&, std::string&) override { return ++next; }
    void BeginFrame(deathleak::render::Color) override {}
    void DrawSprite(const deathleak::render::Sprite&) override {}
    void FillRect(deathleak::render::Rect, deathleak::render::Color) override {}
    void ApplyScenePostprocess() override {}
    void EndFrame() override {}
    bool PollQuit() override { return false; }
    bool KeyDown(int) const override { return false; }
    deathleak::render::MouseState Mouse() const override { return {}; }
    bool SaveFrame(const std::string&, std::string&) override { return false; }
private:
    deathleak::render::TextureId next = 0;
};

int main() {
    using deathleak::game::LevelScene;
    using deathleak::game::PlayerMotion;
    constexpr float dt = 1.f / 60.f;
    // Verify the actual main-loop seam, not merely a copied test loop.
    std::ifstream input("port/linux/main.cpp");
    assert(input);
    const std::string source(std::istreambuf_iterator<char>{input}, {});
    const auto intro = source.find("} else if (!hasPulledSword) {");
    assert(intro != std::string::npos);
    const auto ordinary = source.find("} else {\n            const float direction", intro);
    assert(ordinary != std::string::npos);
    const auto branch = source.substr(intro, ordinary - intro);
    const auto step = branch.find("motion.Update(scene, dt, 0, false);");
    const auto copy = branch.find("worldX = motion.X(); worldY = motion.Y();");
    const auto finish = branch.find("if (scene.IntroBodyFinished())");
    const auto threshold = branch.find("if (introHeldTime > 2.75f)");
    const auto burst = branch.find("scene.EmitIntroSwordPullBurst(*renderer, worldX, worldY);");
    const auto flight = branch.find("combat.SpawnIntroBloodFlight(worldX, worldY + 5.f);");
    const auto follow = branch.find("FollowCameraWithTarget");
    assert(step != std::string::npos && branch.find("motion.Update(", step + 1) == std::string::npos);
    assert(step < copy && copy < finish && finish < threshold && threshold < burst && burst < flight && flight < follow);
    assert(source.find("if (paperModal) {") < source.find("} else if (upgradeModal) {") &&
           source.find("} else if (upgradeModal) {") < intro);
    assert(source.find("if (simulateOnly) {\n            // Keep the intro terminal signal") > ordinary);

    SpyRenderer renderer;
    LevelScene scene;
    std::string error;
    assert(scene.Load("Game/XmlFiles/levels/forest_exploration_1.xml", renderer, error));
    PlayerMotion motion;
    motion.Reset(scene.PlayerX(), scene.PlayerY(), true);
    const float spawnY = motion.Y();
    assert(!motion.Grounded());
    motion.Update(scene, dt, 0, false);
    assert(motion.Y() < spawnY && !motion.Grounded());
    int contactTick = -1;
    for (int tick = 2; tick <= 180; ++tick) {
        motion.Update(scene, dt, 0, false);
        if (motion.Grounded()) { contactTick = tick; break; }
    }
    assert(contactTick > 1 && contactTick < 180);
    const float groundY = motion.Y();
    assert(groundY < spawnY && std::abs(motion.X() - scene.PlayerX()) < .01f);
    for (int tick = 0; tick < 180; ++tick) {
        motion.Update(scene, dt, 0, false);
        assert(motion.Grounded() && std::abs(motion.Y() - groundY) < .01f);
    }
    // Strict source threshold: equality holds; only the next positive step pulls.
    float heldTime = 2.75f - .001f;
    assert(!(heldTime > 2.75f));
    heldTime = 2.75f;
    assert(!(heldTime > 2.75f));
    heldTime += dt;
    assert(heldTime > 2.75f);
    scene.SetIntroBodyPhase(LevelScene::IntroBodyPhase::Looped);
    motion.Update(scene, dt, 0, false); // Pull-threshold tick precedes burst.
    const float burstY = motion.Y();
    scene.EmitIntroSwordPullBurst(renderer, motion.X(), burstY);
    const auto& sprays = scene.IntroSwordPullSprays();
    assert(sprays.size() == 2);
    assert(std::abs(sprays[0].x - (motion.X() - 35.f)) < .01f);
    assert(std::abs(sprays[0].y - (burstY - 15.f)) < .01f);
    assert(std::abs(sprays[1].y - (burstY - 20.f)) < .01f);
    scene.SetIntroBodyPhase(LevelScene::IntroBodyPhase::Pull);
    // Completion tick still takes exactly one zero-input physics step; the next
    // ordinary no-shot tick takes its own step, with no transition double-step.
    motion.Update(scene, dt, 0, false);
    assert(motion.Grounded() && std::abs(motion.Y() - groundY) < .01f);
    scene.SetIntroBodyPhase(LevelScene::IntroBodyPhase::None);
    motion.Update(scene, dt, 0, false);
    assert(motion.Grounded() && std::abs(motion.Y() - groundY) < .01f);
    std::cout << "fresh-save intro physics CPU test passed contact_tick=" << contactTick
              << " spawn_y=" << spawnY << " ground_y=" << groundY << '\n';
}
