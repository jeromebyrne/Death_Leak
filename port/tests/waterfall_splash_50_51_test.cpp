#include "../game/LevelScene.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

class CaptureRenderer final : public deathleak::render::Renderer {
public:
    std::vector<std::string> loaded;
    std::vector<deathleak::render::Sprite> drawn;
    bool Initialize(const char*, int, int, std::string&) override { return true; }
    deathleak::render::TextureId LoadTexture(const std::string& path, std::string&) override {
        loaded.push_back(path); return static_cast<deathleak::render::TextureId>(loaded.size());
    }
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
    setenv("DEATHLEAK_RENDER_ONLY_XML_IDS", "50,51", 1);
    CaptureRenderer renderer;
    deathleak::game::LevelScene scene;
    std::string error;
    assert(scene.Load("Game/XmlFiles/levels/waterfall_top.xml", renderer, error));
    assert(std::count(renderer.loaded.begin(), renderer.loaded.end(), "Game/Media/waterfall_splash.png") == 1);
    const auto splashTexture = static_cast<deathleak::render::TextureId>(
        1 + std::find(renderer.loaded.begin(), renderer.loaded.end(), "Game/Media/waterfall_splash.png") - renderer.loaded.begin());
    constexpr float cx = 333.f, cy = 2703.f;
    scene.UpdateVisuals(1.f/60.f, cx, cy, cx, cy);
    scene.Draw(renderer, cx, cy, cx, cy);
    assert(renderer.drawn.size() == 20); // 10 quads per emitter, no 300x300 bounds quad.
    int left = 0, right = 0;
    for (const auto& sprite : renderer.drawn) {
        assert(sprite.texture == splashTexture);
        assert(sprite.destination.width >= 45.f*2.f/3.f);
        assert(sprite.destination.width <= 90.f*4.f*2.f/3.f);
        assert(sprite.destination.width == sprite.destination.height);
        if (sprite.destination.x < 640.f) ++left; else ++right;
    }
    assert(left == 10 && right == 10);
    renderer.drawn.clear();
    scene.Draw(renderer, 8000.f, 8000.f, 8000.f, 8000.f);
    assert(renderer.drawn.empty());

    // Independently check the existing source-backed negative-speed, drag,
    // gravity, pre-advance draw coordinate, growth and loop-reset mechanics.
    deathleak::game::ParticleSpray spray;
    spray.warmed = true;
    spray.scalesByLifetime = true; spray.scaleTo = 4.f;
    deathleak::game::ParticleSpray::Particle p;
    p.x = p.startX = 10.f; p.y = p.startY = 20.f;
    p.directionX = .01f; p.directionY = -1.f;
    p.speed = p.startSpeed = -32.f;
    p.gravity = 22.f; p.size = p.startSize = 90.f;
    p.lifetime = 1.f;
    spray.particles.push_back(p);
    spray.Update(1.f/60.f);
    const auto& after = spray.particles.front();
    assert(std::abs(after.drawX - 10.f) < 1e-5f && std::abs(after.drawY - 20.f) < 1e-5f);
    assert(std::abs(after.speed - (-31.68f)) < 1e-4f);
    assert(std::abs(after.x - 9.6832f) < 1e-4f);
    assert(std::abs(after.y - 29.68f) < 1e-4f);
    assert(std::abs(after.size - 94.5f) < 1e-4f);
    spray.Update(59.f/60.f);
    const auto& reset = spray.particles.front();
    assert(reset.x == 10.f && reset.y == 20.f && reset.speed == -32.f && reset.size == 90.f && reset.elapsed == 0.f);
    std::cout << "PASS: paired 10-particle waterfall-top splashes and negative-speed lifecycle\n";
}
