#include "../game/Hud.h"
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>
using namespace deathleak;
namespace {
std::uint64_t digest = 1469598103934665603ULL;
void word(std::uint32_t v) { digest = (digest ^ v) * 1099511628211ULL; }
void number(float value) { std::uint32_t bits; std::memcpy(&bits, &value, sizeof(bits)); word(bits); }
struct Spy final : render::Renderer {
    bool Initialize(const char*, int, int, bool, std::string&) override { return true; }
    render::TextureId LoadTexture(const std::string& path, std::string&) override {
        if (path.find("meter_bar_middle_back") != std::string::npos) return 6;
        if (path.find("xp_meter_bar_middle") != std::string::npos) return 3;
        if (path.find("meter_bar_middle") != std::string::npos) return 2;
        if (path.find("meter_edge_2") != std::string::npos) return 1;
        if (path.find("meter_overlay") != std::string::npos) return 4;
        if (path.find("orb_count") != std::string::npos) return 5;
        return 0;
    }
    void BeginFrame(render::Color) override {}
    void DrawSprite(const render::Sprite& s) override {
        word(s.texture); number(s.source.x); number(s.source.y); number(s.source.width); number(s.source.height);
        number(s.destination.x); number(s.destination.y); number(s.destination.width); number(s.destination.height);
        word(s.tint.r); word(s.tint.g); word(s.tint.b); word(s.tint.a);
        number(s.opacity); number(s.rotationDegrees); word(s.flipX); word(s.flipY); word(s.repeatX);
    }
    void FillRect(render::Rect r, render::Color c) override {
        number(r.x); number(r.y); number(r.width); number(r.height);
        word(c.r); word(c.g); word(c.b); word(c.a);
    }
    void ApplyScenePostprocess() override {}
    void EndFrame() override {}
    bool PollQuit() override { return false; }
    bool KeyDown(int) const override { return false; }
    render::MouseState Mouse() const override { return {}; }
    bool SaveFrame(const std::string&, std::string&) override { return false; }
};
}
int main() {
    Spy spy; game::Hud hud; hud.Load(spy);
    for (int frame = 0; frame < 240; ++frame) {
        const float health = frame < 30 ? 25.f : frame < 120 ? 17.f : frame < 170 ? 28.f : 13.f;
        const float maxHealth = frame < 100 ? 25.f : 35.f;
        const float focus = frame < 55 ? 100.f : frame < 165 ? 40.f : 93.f;
        const float t = frame / 60.f;
        const int orbs = frame / 29;
#ifdef BASELINE_HUD
        hud.Draw(spy, health, maxHealth, focus, orbs, t);
#else
        hud.Prepare(health, maxHealth, focus, t);
        hud.Draw(spy, orbs);
#endif
        word(0xffffffffU); // frame boundary
    }
    std::cout << std::hex << digest << '\n';
}
