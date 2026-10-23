#include "../game/ParticleSpray.h"
#include <cassert>
#include <cmath>
#include <string>

using namespace deathleak;

struct SpyRenderer final : render::Renderer {
    render::Sprite submitted{};
    int draws = 0;
    bool Initialize(const char*, int, int, std::string&) override { return true; }
    render::TextureId LoadTexture(const std::string&, std::string&) override { return 1; }
    void BeginFrame(render::Color) override {}
    void DrawSprite(const render::Sprite& sprite) override { submitted = sprite; ++draws; }
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
    spray.texture = 7;
    game::ParticleSpray::Particle particle;
    particle.drawX = 100;
    particle.drawY = 200;
    particle.drawSize = 60;
    particle.size = 60;
    spray.particles.push_back(particle);
    SpyRenderer spy;
    spray.Draw(spy, 100, 200);
    assert(spy.draws == 1 && spy.submitted.texture == 7);
    const auto& s = spy.submitted;
    assert(std::abs(s.destination.x - 620.f) < .001f);
    assert(std::abs(s.destination.y - 340.f) < .001f);
    assert(std::abs(s.destination.width - 40.f) < .001f);
    assert(std::abs(s.destination.height - 40.f) < .001f);
    assert(s.flipX && !s.flipY);
    // Source vertices: world (-x,-y)->(U,V)=(1,1), (+x,-y)->(0,1),
    // (+x,+y)->(0,0), (-x,+y)->(1,0). Screen top is world +y.
    const float screenLeftU = s.flipX ? 1.f : 0.f;
    const float screenRightU = 1.f - screenLeftU;
    const float screenTopV = s.flipY ? 1.f : 0.f;
    const float screenBottomV = 1.f - screenTopV;
    assert(screenLeftU == 1.f && screenRightU == 0.f);
    assert(screenTopV == 0.f && screenBottomV == 1.f);
}
