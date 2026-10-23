// CPU-only source animation cadence and terminal contract for forest-1 Player ID2.
#include "../game/LevelScene.h"
#include <cassert>
#include <iostream>
#include <string>

class NullRenderer final : public deathleak::render::Renderer {
public:
    bool Initialize(const char*, int, int, std::string&) override { return true; }
    deathleak::render::TextureId LoadTexture(const std::string&, std::string&) override { return ++next_; }
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
    deathleak::render::TextureId next_ = 0;
};

int main() {
    using Scene = deathleak::game::LevelScene;
    NullRenderer renderer;
    Scene scene;
    std::string error;
    assert(scene.Load("Game/XmlFiles/levels/forest_exploration_1.xml", renderer, error));
    auto tick = [&](float prior = 1.f / 60.f) {
        scene.UpdatePlayerVisual(1.f / 60.f, 0, 0, true, 0, false, false, false,
                                 0, 1, false, false, {}, false, 0, 0, prior);
    };
    assert(scene.GetIntroBodyPhase() == Scene::IntroBodyPhase::None);
    scene.SetIntroBodyPhase(Scene::IntroBodyPhase::Looped);
    assert(scene.IntroBodyFrameCount() == 10 && scene.IntroBodyFrame() == 0);
    for (int i = 0; i < 3; ++i) tick();
    assert(scene.IntroBodyFrame() == 0); // 15 fps: four 60-Hz ticks/frame.
    tick(); assert(scene.IntroBodyFrame() == 1);
    for (int i = 0; i < 36; ++i) tick();
    assert(scene.IntroBodyFrame() == 0 && !scene.IntroBodyFinished());
    scene.SetIntroBodyPhase(Scene::IntroBodyPhase::Pull);
    assert(scene.IntroBodyFrameCount() == 20 && scene.IntroBodyFrame() == 0);
    tick(0.f); assert(scene.IntroBodyFrame() == 0); // Preceding rendered delta wins over dt.
    for (int i = 0; i < 38; ++i) tick();
    assert(scene.IntroBodyFrame() == 19 && !scene.IntroBodyFinished());
    tick(); assert(scene.IntroBodyFrame() == 19 && !scene.IntroBodyFinished());
    tick(); assert(scene.IntroBodyFrame() == 20 && scene.IntroBodyFinished());
    for (int i = 0; i < 20; ++i) tick();
    assert(scene.IntroBodyFrame() == 20 && scene.IntroBodyFinished());
    scene.SetIntroBodyPhase(Scene::IntroBodyPhase::None);
    assert(!scene.IntroBodyFinished());
    std::cout << "fresh-save intro animation CPU test passed\n";
}
