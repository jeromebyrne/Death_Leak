#include "../game/ParticleSpray.h"
#include <cassert>
#include <string>
#include <vector>

using namespace deathleak;

struct SpyRenderer final : render::Renderer {
    std::vector<render::Sprite> submitted;
    bool Initialize(const char*, int, int, std::string&) override { return true; }
    render::TextureId LoadTexture(const std::string&, std::string&) override { return 1; }
    void BeginFrame(render::Color) override {}
    void DrawSprite(const render::Sprite& sprite) override { submitted.push_back(sprite); }
    void FillRect(render::Rect, render::Color) override {}
    void ApplyScenePostprocess() override {}
    void EndFrame() override {}
    bool PollQuit() override { return false; }
    bool KeyDown(int) const override { return false; }
    render::MouseState Mouse() const override { return {}; }
    bool SaveFrame(const std::string&, std::string&) override { return false; }
};

int main() {
    game::ParticleSpray spray;
    spray.texture = 17;
    game::ParticleSpray::Particle visible;
    visible.drawX = 0;
    visible.drawY = 0;
    visible.drawSize = 90;
    visible.brightness = 1;
    visible.alpha = 1;
    spray.particles.push_back(visible);
    auto dead = visible;
    dead.dead = true;
    spray.particles.push_back(dead);
    auto culled = visible;
    culled.drawX = 3000;
    spray.particles.push_back(culled);

    SpyRenderer spy;
    spray.Draw(spy, 0, 0);
    assert(spy.submitted.size() == 1);
    const auto& leaf = spy.submitted.front();
    assert(leaf.texture == 17);
    assert(leaf.flipX && !leaf.flipY);
    assert(leaf.destination.width == 60 && leaf.destination.height == 60);
    assert(leaf.tint.a == 255);
}
