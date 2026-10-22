#include "../game/ParticleSpray.h"
#include <cassert>
#include <iostream>
#include <vector>

using namespace deathleak;

struct RecordingRenderer final : render::Renderer {
    std::vector<render::Sprite> draws;
    bool Initialize(const char*, int, int, std::string&) override { return true; }
    render::TextureId LoadTexture(const std::string&, std::string&) override { return 1; }
    void BeginFrame(render::Color) override {}
    void DrawSprite(const render::Sprite& sprite) override { draws.push_back(sprite); }
    void FillRect(render::Rect, render::Color) override {}
    void ApplyScenePostprocess() override {}
    void EndFrame() override {}
    bool PollQuit() override { return false; }
    bool KeyDown(int) const override { return false; }
    render::MouseState Mouse() const override { return {}; }
    bool SaveFrame(const std::string&, std::string&) override { return false; }
};

int main() {
    RecordingRenderer renderer;
    game::ParticleSpray spray;
    spray.texture = 1;
    game::ParticleSpray::Particle leaf;
    leaf.drawX = leaf.drawY = 0;
    leaf.size = 60;
    spray.particles.push_back(leaf);

    const auto check = [&](float brightness, float fade, int rgb, int alpha) {
        spray.particles[0].brightness = brightness;
        spray.particles[0].alpha = fade;
        renderer.draws.clear();
        spray.Draw(renderer, 0, 0);
        assert(renderer.draws.size() == 1);
        const auto& tint = renderer.draws[0].tint;
        assert(tint.r == rgb && tint.g == rgb && tint.b == rgb);
        assert(tint.a == alpha);
    };
    // Particle.fx: sampled RGBA * brightness, then alpha * fade.
    check(.85f, 1.f, 217, 217);
    check(.5f, .5f, 128, 64);
    check(1.f, .5f, 255, 128);
    check(1.2f, 1.2f, 255, 255);
    check(.85f, 0.f, 217, 0);
    std::cout << "PASS: particle RGB brightness and brightness*fade alpha\n";
}
