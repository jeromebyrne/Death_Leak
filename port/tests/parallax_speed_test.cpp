// Source-backed camera/parallax displacement check; build/run from source/.
#include "../game/LevelScene.h"
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <unordered_map>

class CaptureRenderer final : public deathleak::render::Renderer {
public:
    std::vector<deathleak::render::Sprite> drawn;
    bool Initialize(const char*, int, int, std::string&) override { return true; }
    deathleak::render::TextureId LoadTexture(const std::string&, std::string&) override { return 1; }
    void BeginFrame(deathleak::render::Color) override {}
    void DrawSprite(const deathleak::render::Sprite& s) override { drawn.push_back(s); }
    void FillRect(deathleak::render::Rect, deathleak::render::Color) override {}
    void ApplyScenePostprocess() override {}
    void EndFrame() override {}
    bool PollQuit() override { return false; }
    bool KeyDown(int) const override { return false; }
    deathleak::render::MouseState Mouse() const override { return {}; }
    bool SaveFrame(const std::string&, std::string&) override { return false; }
};

int main() {
    setenv("DEATHLEAK_RENDER_ONLY_XML_IDS", "1,37,40", 1);
    CaptureRenderer renderer;
    deathleak::game::LevelScene scene;
    std::string error;
    assert(scene.Load("Game/XmlFiles/levels/forest_exploration_1.xml", renderer, error));
    auto at = [&](float cameraX) {
        // Compare settled cameras: each eligible update first builds the world
        // matrix from the preceding followed X, then stores this camera X.
        scene.UpdateVisuals(1.f/60.f, scene.PlayerX(), scene.PlayerY(), cameraX, 3400.f);
        scene.UpdateVisuals(1.f/60.f, scene.PlayerX(), scene.PlayerY(), cameraX, 3400.f);
        renderer.drawn.clear();
        scene.Draw(renderer, cameraX, 3400.f, 0.f, 3400.f);
        assert(renderer.drawn.size() == 3);
        return renderer.drawn;
    };
    const auto before = at(0.f), after = at(100.f);
    auto near = [](float a, float b) { assert(std::abs(a-b) < .001f); };
    // Original ParallaxLayer::Draw: left UV = 1 - cameraX*m, right UV = -cameraX*m.
    // A feature moves left by quadWidth*m per world camera unit, despite the
    // camera-following quad itself remaining stationary.
    near(after[0].destination.x - before[0].destination.x, 0.f);
    near(after[0].uvShiftX - before[0].uvShiftX, 100.f * .000059f);
    near(after[1].uvShiftX - before[1].uvShiftX, 100.f * .000690f);
    near(after[2].uvShiftX - before[2].uvShiftX, 100.f * .000690f);
    near((after[0].uvShiftX-before[0].uvShiftX)*before[0].destination.width, 100.f*.000059f*1985.f*(2.f/3.f));
    near((after[1].uvShiftX-before[1].uvShiftX)*before[1].destination.width, 100.f*.000690f*2048.f*(2.f/3.f));
    std::cout << "PASS: forest background 0.117115 and foreground 1.41312 world px/camera px, leftward\n";
}
