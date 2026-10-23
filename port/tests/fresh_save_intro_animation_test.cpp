// CPU command-path and authored animation-clock oracle for forest-1 Player ID2.
#include "../game/LevelScene.h"
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

class SpyRenderer final : public deathleak::render::Renderer {
public:
    std::unordered_map<deathleak::render::TextureId, std::string> paths;
    std::vector<deathleak::render::Sprite> drawn;
    bool Initialize(const char*, int, int, std::string&) override { return true; }
    deathleak::render::TextureId LoadTexture(const std::string& path, std::string&) override {
        const auto id = ++next_;
        paths[id] = path;
        return id;
    }
    void BeginFrame(deathleak::render::Color) override {}
    void DrawSprite(const deathleak::render::Sprite& sprite) override { drawn.push_back(sprite); }
    void FillRect(deathleak::render::Rect, deathleak::render::Color) override {}
    void ApplyScenePostprocess() override {}
    void EndFrame() override {}
    bool PollQuit() override { return false; }
    bool KeyDown(int) const override { return false; }
    deathleak::render::MouseState Mouse() const override { return {}; }
    bool SaveFrame(const std::string&, std::string&) override { return false; }
private:
    deathleak::render::TextureId next_ = 0;
};

int main() {
    using Scene = deathleak::game::LevelScene;
    // Exclude every effect/object other than ID2, so a particle cannot satisfy an arm assertion.
    setenv("DEATHLEAK_RENDER_ONLY_XML_IDS", "2", 1);
    SpyRenderer renderer;
    Scene scene;
    std::string error;
    assert(scene.Load("Game/XmlFiles/levels/forest_exploration_1.xml", renderer, error));
    const float x = scene.PlayerX(), y = scene.PlayerY();
    const auto draw = [&](const char* bodySequence, size_t frame, bool expectArm) {
        renderer.drawn.clear();
        scene.Draw(renderer, x, y, x, y);
        const std::string bodyPrefix = std::string("Game/Media/characters/player/") + bodySequence + "/" + bodySequence + "_";
        int armIndex = -1, bodyIndex = -1;
        for (size_t i = 0; i < renderer.drawn.size(); ++i) {
            const auto& path = renderer.paths.at(renderer.drawn[i].texture);
            if (path.find("Game/Media/characters/player/back_arm_") == 0) {
                assert(armIndex < 0); armIndex = static_cast<int>(i);
            } else {
                assert(path.find(bodyPrefix) == 0);
                assert(bodyIndex < 0); bodyIndex = static_cast<int>(i);
                const std::string suffix = (frame < 10 ? "00" : "0") + std::to_string(frame) + ".png";
                assert(path == bodyPrefix + suffix);
            }
        }
        assert(bodyIndex >= 0);
        assert(expectArm ? armIndex >= 0 && armIndex + 1 == bodyIndex : armIndex < 0);
    };
    const auto tick = [&](float prior, bool fired = false) {
        scene.UpdatePlayerVisual(1.f / 60.f, 0, 0, true, 0, false, false, false,
                                 0, 1, fired, false, {}, false, 0, 0, prior);
    };
    assert(scene.GetIntroBodyPhase() == Scene::IntroBodyPhase::None);
    scene.SetIntroBodyPhase(Scene::IntroBodyPhase::Looped);
    assert(scene.IntroBodyFrameCount() == 10 && scene.IntroBodyFrame() == 0);
    float elapsed = 0;
    size_t expected = 0;
    for (int i = 0; i < 44; ++i) {
        const float prior = i == 0 ? 0.f : 1.f / 60.f;
        tick(prior);
        elapsed += prior;
        if (elapsed >= 1.f / 15.f) { elapsed -= 1.f / 15.f; expected = (expected + 1) % 10; }
        assert(scene.IntroBodyFrame() == expected && !scene.IntroBodyFinished());
        draw("intro_cutscene_1", expected, false);
    }
    const auto pull = [&](const std::vector<float>& deltas) {
        scene.SetIntroBodyPhase(Scene::IntroBodyPhase::Pull);
        assert(scene.IntroBodyFrameCount() == 20 && scene.IntroBodyFrame() == 0);
        float clock = 0;
        size_t index = 0;
        bool finished = false;
        draw("intro_cutscene_2", 0, false); // Transition draw also suppresses the arm.
        for (float prior : deltas) {
            tick(prior);
            if (!finished) {
                clock += prior;
                if (clock >= 1.f / 40.f) {
                    clock -= 1.f / 40.f; // Original Animate: at most one advancement/update.
                    if (++index == 20) finished = true;
                }
            }
            assert(scene.IntroBodyFrame() == index);
            assert(scene.IntroBodyFinished() == finished);
            draw("intro_cutscene_2", index == 20 ? 19 : index, false);
        }
        assert(finished);
    };
    std::vector<float> nominal(34, 1.f / 60.f);
    nominal[0] = 0.f; // Current dt is positive; preceding rendered modified delta wins.
    pull(nominal);
    scene.SetIntroBodyPhase(Scene::IntroBodyPhase::Looped);
    tick(0.f); draw("intro_cutscene_1", 0, false);
    std::vector<float> variable{0.f, .005f, .04f, .002f, .08f, .001f};
    for (int i = 0; i < 45; ++i) variable.push_back(i % 3 == 0 ? .04f : .015f);
    pull(variable);
    scene.SetIntroBodyPhase(Scene::IntroBodyPhase::None);
    assert(!scene.IntroBodyFinished());
    tick(0.f); // First ordinary no-shot update synchronizes the finished intro arm to Still.
    draw("idle_default", 0, false);
    float idleClock = 0.f;
    size_t idleFrame = 0;
    for (int i = 0; i < 5; ++i) {
        tick(1.f / 60.f);
        idleClock += 1.f / 60.f;
        if (idleClock >= 1.f / 20.f) { idleClock -= 1.f / 20.f; ++idleFrame; }
        draw("idle_default", idleFrame, false); // Ordinary idle must not resurrect the arm.
    }
    tick(0.f, true); // A later successful shot restarts the ordinary Still arm.
    draw("idle_default", idleFrame, true); // Arm remains before body.
    std::cout << "fresh-save intro animation CPU test passed\n";
}
