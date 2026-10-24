#include "../game/LevelScene.h"
#include "../linux/WobbleUV.h"
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>

using namespace deathleak;
class CaptureRenderer final : public render::Renderer {
public:
    std::vector<render::Sprite> drawn;
    bool Initialize(const char*, int, int, std::string&) override { return true; }
    render::TextureId LoadTexture(const std::string&, std::string&) override { return 1; }
    void BeginFrame(render::Color) override {}
    void DrawSprite(const render::Sprite& s) override { drawn.push_back(s); }
    void FillRect(render::Rect, render::Color) override {}
    void ApplyScenePostprocess() override {}
    void EndFrame() override {}
    bool PollQuit() override { return false; }
    bool KeyDown(int) const override { return false; }
    render::MouseState Mouse() const override { return {}; }
    bool SaveFrame(const std::string&, std::string&) override { return false; }
};
static void near(float a, float b, float tolerance = 3e-4f) {
    assert(std::fabs(a - b) < tolerance);
}
static float fract(float value) { return value - std::floor(value); }
int main() {
    setenv("DEATHLEAK_RENDER_ONLY_XML_IDS", "6,13,14", 1);
    CaptureRenderer renderer;
    game::LevelScene scene;
    std::string error;
    assert(scene.Load("Game/XmlFiles/levels/waterfall_top.xml", renderer, error));
    constexpr float ordinaryStep = 7.f * (720.f / 1080.f) * .001f;
    constexpr float wobbleStep = 11.f * (720.f / 1080.f) * .001f;
    int wobbleSeenPastWrap = 0, ordinarySeenPastWrap = 0;
    int drawIndex = 0;
    for (int tick = 0; tick < 240; ++tick) {
        scene.UpdateVisuals(1.f / 60.f, scene.PlayerX(), scene.PlayerY(),
                            scene.CameraX(), scene.CameraY());
        for (float cameraY : {2600.f, 3000.f}) {
            renderer.drawn.clear();
            scene.Draw(renderer, 300.f, cameraY, 300.f, cameraY);
            const float expected13 = drawIndex * wobbleStep;
            const float expected6 = fract(drawIndex * ordinaryStep);
            int wobbleCount = 0, ordinaryCount = 0;
            for (const auto& sprite : renderer.drawn) {
                if (sprite.pixelWobble.enabled) {
                    ++wobbleCount;
                    near(sprite.uvShiftY, expected13);
                    if (expected13 > 1.44f && expected13 < 1.46f) {
                        const auto vertex = render::PixelWobbleVertexUV(
                            sprite.flipX, sprite.flipY, sprite.uvShiftX,
                            sprite.uvShiftY, false, true);
                        near(vertex.v, sprite.uvShiftY);
                        const float clock = 10.f;
                        const float sourceU = .005f * std::sin((vertex.v + .2f) * clock);
                        const float reducedU = .005f * std::sin((fract(vertex.v) + .2f) * clock);
                        assert(std::fabs(sourceU - reducedU) > .001f);
                        ++wobbleSeenPastWrap;
                    }
                } else {
                    ++ordinaryCount;
                    near(sprite.uvShiftY, expected6);
                    if (drawIndex * ordinaryStep > 1.f) ++ordinarySeenPastWrap;
                }
            }
            if (cameraY == 2600.f) assert(wobbleCount == 2);
            if (cameraY == 3000.f) assert(ordinaryCount == 1);
            ++drawIndex;
        }
    }
    assert(wobbleSeenPastWrap > 0 && ordinarySeenPastWrap > 0);
    std::cout << "PASS: wobble IDs 13/14 retain unbounded V across integer wrap; ordinary ID6 remains bounded\n";
}
