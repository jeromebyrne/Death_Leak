#include "../game/LoadingOverlayPacket.h"
#include <cassert>
#include <string>
using namespace deathleak;
struct SpyRenderer final : render::Renderer {
    int sprites = 0, fills = 0;
    render::Color last{};
    bool Initialize(const char*, int, int, bool, std::string&) override { return true; }
    render::TextureId LoadTexture(const std::string&, std::string&) override { return 0; }
    void BeginFrame(render::Color) override {}
    void DrawSprite(const render::Sprite& s) override { ++sprites; last = s.tint; }
    void FillRect(render::Rect, render::Color c) override { ++fills; last = c; }
    void ApplyScenePostprocess() override {}
    void EndFrame() override {}
    bool PollQuit() override { return false; }
    bool KeyDown(int) const override { return false; }
    render::MouseState Mouse() const override { return {}; }
    bool SaveFrame(const std::string&, std::string&) override { return true; }
};
int main() {
    using game::LoadingOverlayPacket;
    SpyRenderer r;
    const auto absent = LoadingOverlayPacket::Prepare(false, false, false, false, 0.f, .2f);
    absent.Draw(r, 5); assert(!absent.visible && absent.alpha == -1.f && absent.tintAlpha == -1 && r.sprites == 0);
    const auto in = LoadingOverlayPacket::Prepare(true, false, true, false, .05f, .2f);
    assert(in.alpha == .25f && in.tintAlpha == 64);
    const auto hold = LoadingOverlayPacket::Prepare(true, true, false, true, .15f, .2f);
    assert(hold.alpha == 1.f && hold.tintAlpha == 255);
    const auto out = LoadingOverlayPacket::Prepare(true, false, false, true, .15f, .2f);
    assert(out.alpha > .249f && out.alpha < .251f && out.tintAlpha == 64);
    const auto& presentation = out;
    for (int i = 0; i < 100000; ++i) {
        presentation.Draw(r, 5);
        assert(presentation.alpha == out.alpha && presentation.tintAlpha == 64);
        assert(r.last.a == 64);
    }
    assert(r.sprites == 100000 && r.fills == 0);
    out.Draw(r, 0); assert(r.fills == 1 && r.last.a == 64);
}
