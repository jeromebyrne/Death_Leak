#include "../game/LevelScene.h"
#include "../render/ViewportCull.h"
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>

class CaptureRenderer final : public deathleak::render::Renderer {
public:
    std::vector<deathleak::render::Sprite> drawn;
    bool Initialize(const char*, int, int, std::string&) override { return true; }
    deathleak::render::TextureId LoadTexture(const std::string&, std::string&) override { return 1; }
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
    using deathleak::render::SpriteMayIntersectViewport;
    setenv("DEATHLEAK_RENDER_ONLY_XML_IDS", "117", 1);
    CaptureRenderer renderer;
    deathleak::game::LevelScene scene;
    std::string error;
    assert(scene.Load("Game/XmlFiles/levels/waterfall_top.xml", renderer, error));
    // Advance the authored fog fade while the player remains in its update zone.
    for (int i = 0; i < 6; ++i)
        scene.UpdateVisuals(1.f, -1675.014648f, 2873.f, -1675.014648f, 2873.f);
    auto at = [&](float cameraY) {
        renderer.drawn.clear();
        scene.Draw(renderer, -1675.014648f, cameraY, -1675.014648f, 2873.f);
        return renderer.drawn;
    };
    const auto visible = at(4077.5f); // top of unrotated rect >720; rotated corner <720.
    assert(visible.size() == 1);
    const auto& fog = visible.front();
    assert(fog.destination.y > 720.f);
    assert(std::abs(fog.rotationDegrees + .11f * 180.f / 3.14159265358979323846f) < .001f);
    assert(fog.pixelWobble.enabled && fog.opacity > 0.f);
    assert(SpriteMayIntersectViewport(fog));
    assert(at(4185.f).empty()); // rotated quad fully beyond bottom edge.
    // Unrotated control preserves the old AABB acceptance/rejection semantics.
    auto plain = fog;
    plain.rotationDegrees = 0.f;
    assert(!SpriteMayIntersectViewport(plain));
    plain.destination.y = 719.f;
    assert(SpriteMayIntersectViewport(plain));
    plain.destination.y = 721.f;
    assert(!SpriteMayIntersectViewport(plain));
    std::cout << "PASS: authored ID117 rotated fringe admitted, offscreen rejected, unrotated control\n";
}
