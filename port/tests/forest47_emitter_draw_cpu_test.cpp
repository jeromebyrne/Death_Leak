#include "../game/ParticleSpray.h"
#include <cassert>
#include <cmath>
#include <string>
using namespace deathleak;

struct SpyRenderer final : render::Renderer {
    int draws = 0;
    bool Initialize(const char*, int, int, std::string&) override { return true; }
    render::TextureId LoadTexture(const std::string&, std::string&) override { return 1; }
    void BeginFrame(render::Color) override {}
    void DrawSprite(const render::Sprite&) override { ++draws; }
    void FillRect(render::Rect, render::Color) override {}
    void ApplyScenePostprocess() override {}
    void EndFrame() override {}
    bool PollQuit() override { return false; }
    bool KeyDown(int) const override { return false; }
    render::MouseState Mouse() const override { return {}; }
    bool SaveFrame(const std::string&, std::string&) override { return false; }
};

// Independent transcription of the source Camera2D predicate using scaled
// GameObject coordinates and original 1280x720 camera dimensions.
static bool SourceAdmits(const game::ParticleSpray& s, float cameraX, float cameraY) {
    const float k = 2.f/3.f;
    const float left = cameraX*k - 640.f, right = cameraX*k + 640.f;
    const float bottom = cameraY*k - 360.f, top = cameraY*k + 360.f;
    const float x = s.x*k, y = s.y*k;
    if (x > left && x < right && y > bottom && y < top) return true;
    return x+s.width*k*.5f > left && x-s.width*k*.5f < right &&
           y+s.height*k*.5f > bottom && y-s.height*k*.5f < top;
}

int main() {
    game::ParticleSpray spray;
    spray.xmlId = "47";
    spray.x = 201.684967f; spray.y = 4334.996094f;
    spray.width = 50000.269531f; spray.height = 2000.207764f;
    spray.texture = 1;
    spray.warmed = true;
    // A possible warmed leaf: 4335 - (4 world units/tick * 60 ticks/s *
    // 6.8 seconds) ~= 2703. Four fixed states replace RNG for this oracle.
    for (int i=0; i<4; ++i) {
        game::ParticleSpray::Particle p;
        p.startX = p.x = p.drawX = spray.x;
        p.startY = spray.y;
        p.y = p.drawY = i == 0 ? 2703.f : 4335.f;
        p.size = p.drawSize = 80.f;
        p.speed = p.startSpeed = 4.f;
        p.directionY = -1.f;
        p.lifetime = 40.f; p.elapsed = i == 0 ? 6.8f : 0.f;
        spray.particles.push_back(p);
    }
    SpyRenderer spy;
    const float boundary = spray.y - spray.height*.5f - 540.f;
    const float offCameraY = boundary - 1.f;
    const float onCameraY = boundary + 1.f;
    assert(!SourceAdmits(spray, spray.x, offCameraY));
    assert(!spray.Forest47EmitterInView(spray.x, offCameraY));
    // Existing per-leaf draw would submit the visible warm leaf here.
    spray.Draw(spy, spray.x, offCameraY);
    assert(spy.draws == 1);
    spy.draws = 0;
    if (spray.Forest47EmitterInView(spray.x, offCameraY)) spray.Draw(spy, spray.x, offCameraY);
    assert(spy.draws == 0);
    assert(SourceAdmits(spray, spray.x, onCameraY));
    assert(spray.Forest47EmitterInView(spray.x, onCameraY));
    if (spray.Forest47EmitterInView(spray.x, onCameraY)) spray.Draw(spy, spray.x, onCameraY);
    assert(spy.draws == 1);
    // Strict source edge: contact alone is not overlap.
    assert(!spray.Forest47EmitterInView(spray.x, boundary));
}
