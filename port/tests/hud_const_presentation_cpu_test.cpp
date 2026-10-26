#include "../game/Hud.h"
#include <cassert>
#include <cmath>
#include <string>
#include <vector>

using namespace deathleak;
namespace {
struct Spy final : render::Renderer {
    std::vector<render::Sprite> sprites;
    std::vector<render::Rect> rects;
    bool Initialize(const char*, int, int, bool, std::string&) override { return true; }
    render::TextureId LoadTexture(const std::string& path, std::string&) override {
        if (path.find("meter_bar_middle_back") != std::string::npos) return 6;
        if (path.find("xp_meter_bar_middle") != std::string::npos) return 3;
        if (path.find("meter_bar_middle") != std::string::npos) return 2;
        if (path.find("meter_edge_2") != std::string::npos) return 1;
        if (path.find("meter_overlay") != std::string::npos) return 4;
        if (path.find("orb_count") != std::string::npos) return 5;
        assert(false); return 0;
    }
    void BeginFrame(render::Color) override {}
    void DrawSprite(const render::Sprite& sprite) override { sprites.push_back(sprite); }
    void FillRect(render::Rect rect, render::Color) override { rects.push_back(rect); }
    void ApplyScenePostprocess() override {}
    void EndFrame() override {}
    bool PollQuit() override { return false; }
    bool KeyDown(int) const override { return false; }
    render::MouseState Mouse() const override { return {}; }
    bool SaveFrame(const std::string&, std::string&) override { return false; }
    void clear() { sprites.clear(); rects.clear(); }
};
float width(const Spy& spy, render::TextureId texture) {
    for (const auto& sprite : spy.sprites) if (sprite.texture == texture) return sprite.destination.width;
    assert(false); return 0;
}
bool same(const Spy& a, const Spy& b) {
    if (a.sprites.size() != b.sprites.size() || a.rects.size() != b.rects.size()) return false;
    for (size_t i = 0; i < a.sprites.size(); ++i) {
        const auto& x = a.sprites[i]; const auto& y = b.sprites[i];
        if (x.texture != y.texture || x.destination.x != y.destination.x ||
            x.destination.y != y.destination.y || x.destination.width != y.destination.width ||
            x.destination.height != y.destination.height || x.tint.r != y.tint.r ||
            x.tint.g != y.tint.g || x.tint.b != y.tint.b || x.tint.a != y.tint.a ||
            x.flipX != y.flipX || x.flipY != y.flipY) return false;
    }
    for (size_t i = 0; i < a.rects.size(); ++i) {
        const auto& x = a.rects[i]; const auto& y = b.rects[i];
        if (x.x != y.x || x.y != y.y || x.width != y.width || x.height != y.height) return false;
    }
    return true;
}
}
int main() {
    Spy first, repeated, afterUpdate;
    game::Hud hud;
    hud.Load(first);
    hud.Prepare(25, 25, 100, 0);
    hud.Prepare(25, 25, 100, 1.f/60);
    hud.Prepare(20, 25, 50, 1);
    hud.Prepare(20, 25, 50, 1.25f);
    hud.Draw(first, 12);
    const game::Hud& readOnlyHud = hud;
    readOnlyHud.Draw(repeated, 12);
    assert(same(first, repeated));
    assert(width(first, 6) > width(first, 2));
    const float previousBackingWidth = width(first, 6);
    hud.Prepare(20, 25, 50, 1.26f);
    readOnlyHud.Draw(afterUpdate, 12);
    assert(std::fabs(previousBackingWidth - width(afterUpdate, 6) - (75.f - 3.2f)*2.f/3.f*.005f) < .001f);
    assert(!same(first, afterUpdate));
}
