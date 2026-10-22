// Source-authored CurrencyOrb frame clock freezes outside GameObjectManager's zone.
#include "../game/LevelScene.h"
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

class CaptureRenderer final : public deathleak::render::Renderer {
public:
    std::unordered_map<std::string, deathleak::render::TextureId> ids;
    std::vector<deathleak::render::Sprite> drawn;
    bool Initialize(const char*, int, int, std::string&) override { return true; }
    deathleak::render::TextureId LoadTexture(const std::string& path, std::string&) override {
        auto [it, inserted] = ids.emplace(path, static_cast<deathleak::render::TextureId>(ids.size() + 1));
        return it->second;
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
};

int main() {
    setenv("DEATHLEAK_RENDER_ONLY_XML_IDS", "68", 1);
    CaptureRenderer renderer;
    deathleak::game::LevelScene scene;
    std::string error;
    assert(scene.Load("Game/XmlFiles/levels/forest_exploration_1.xml", renderer, error));
    constexpr float x = 53.985352f, y = 3206.f, halfWidth = 47.697300f / 2.f;
    const auto first = renderer.ids.at("Game/Media/orb/1.png");
    const auto second = renderer.ids.at("Game/Media/orb/11.png");
    const auto third = renderer.ids.at("Game/Media/orb/21.png");
    auto frame = [&]() {
        renderer.drawn.clear();
        scene.Draw(renderer, x, y, x, y);
        assert(renderer.drawn.size() == 1);
        return renderer.drawn.front().texture;
    };
    assert(frame() == first);
    scene.UpdateVisuals(.06f, x, y, x, y); // Fractional clock, not yet a frame.
    assert(frame() == first);
    const float outside = x + 2000.f + halfWidth + .1f;
    scene.UpdateVisuals(1.f, x, y, outside, y); // Fully beyond the zone.
    assert(frame() == first);
    scene.UpdateVisuals(.05f, x, y, outside - .2f, y); // Re-enter with saved .06 s.
    assert(frame() == second);
    scene.UpdateVisuals(1.f, x, y, outside, y);
    assert(frame() == second);
    scene.UpdateVisuals(.1f, x, y, x, y);
    assert(frame() == third); // One frame at most per update, no off-zone catch-up.
    std::cout << "PASS: CurrencyOrb update-zone freezes frame and fractional clock\n";
}
