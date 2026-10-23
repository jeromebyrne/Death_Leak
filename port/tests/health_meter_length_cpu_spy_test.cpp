#include "../game/Hud.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

using namespace deathleak;
namespace {
bool near(float a, float b) { return std::fabs(a - b) < .01f; }
struct Spy final : render::Renderer {
    std::vector<render::Sprite> sprites;
    std::vector<render::Rect> glyphs;
    bool Initialize(const char*, int, int, std::string&) override { return true; }
    render::TextureId LoadTexture(const std::string& path, std::string&) override {
        if (path.find("meter_edge_2") != std::string::npos) return 1;
        if (path.find("xp_meter_bar_middle") != std::string::npos) return 3;
        if (path.find("meter_bar_middle_back") != std::string::npos) return 6;
        if (path.find("meter_bar_middle") != std::string::npos) return 2;
        if (path.find("meter_overlay") != std::string::npos) return 4;
        if (path.find("orb_count") != std::string::npos) return 5;
        assert(false); return 0;
    }
    void BeginFrame(render::Color) override {}
    void DrawSprite(const render::Sprite& s) override { sprites.push_back(s); }
    void FillRect(render::Rect r, render::Color) override { glyphs.push_back(r); }
    void ApplyScenePostprocess() override {}
    void EndFrame() override {}
    bool PollQuit() override { return false; }
    bool KeyDown(int) const override { return false; }
    render::MouseState Mouse() const override { return {}; }
    bool SaveFrame(const std::string&, std::string&) override { return false; }
    void clear() { sprites.clear(); glyphs.clear(); }
    const render::Sprite& at(render::TextureId id, int occurrence = 0) const {
        for (const auto& s : sprites)
            if (s.texture == id && occurrence-- == 0) return s;
        assert(false); return sprites.front();
    }
};
void checkHealth(const Spy& spy, float sourceLength, float progress) {
    const float projected = sourceLength * 2.f / 3.f;
    assert(near(spy.at(4).destination.x, 40.f));
    assert(near(spy.at(4).destination.width, projected));
    assert(near(spy.at(1, 1).destination.x, 37.f + projected));
    assert(spy.at(1, 1).flipX);
    assert(near(spy.at(2).destination.width, (sourceLength - 3.2f) * 2.f / 3.f * progress));
}
void checkUnchanged(const Spy& spy, const std::vector<render::Sprite>& reference,
                    const std::vector<render::Rect>& referenceGlyphs) {
    assert(spy.sprites.size() == reference.size());
    for (std::size_t i = 0; i < reference.size(); ++i) {
        const auto& a = spy.sprites[i]; const auto& b = reference[i];
        if (a.texture == 2 || (a.texture == 4 && a.destination.y == 43) ||
            (a.texture == 1 && a.destination.y == 42 && a.flipX)) continue;
        assert(a.texture == b.texture && a.flipX == b.flipX);
        assert(near(a.destination.x,b.destination.x) && near(a.destination.y,b.destination.y));
        assert(near(a.destination.width,b.destination.width) && near(a.destination.height,b.destination.height));
        assert(a.tint.a == b.tint.a);
    }
    assert(spy.glyphs.size() == referenceGlyphs.size());
    for (std::size_t i = 0; i < referenceGlyphs.size(); ++i)
        assert(near(spy.glyphs[i].x,referenceGlyphs[i].x) && near(spy.glyphs[i].y,referenceGlyphs[i].y));
}
}
int main() {
    Spy spy;
    game::Hud hud;
    hud.Load(spy);
    hud.Draw(spy, 25, 25, 100, 12, 0.f);
    checkHealth(spy, 300, 1); // XML width on first cap-observation update
    const auto reference = spy.sprites;
    const auto referenceGlyphs = spy.glyphs;
    spy.clear();
    hud.Draw(spy, 25, 25, 100, 12, 1.f / 60.f);
    checkHealth(spy, 75, 1); // next update immediately shrinks
    checkUnchanged(spy, reference, referenceGlyphs);
    spy.clear();
    hud.Draw(spy, 12.5f, 25, 100, 12, 2.f / 60.f);
    checkHealth(spy, 75, 1.f); // target registration follows meter update

    game::Hud imported;
    imported.Load(spy);
    spy.clear();
    imported.Draw(spy, 65, 65, 100, 12, 0.f);
    checkHealth(spy, 300, 1);
    spy.clear();
    imported.Draw(spy, 65, 65, 100, 12, 1.f / 60.f);
    checkHealth(spy, 195, 1);
    checkUnchanged(spy, reference, referenceGlyphs);
    spy.clear();
    imported.Draw(spy, 32.5f, 65, 100, 12, 2.f / 60.f);
    checkHealth(spy, 195, 1.f); // registration tick

    spy.clear();
    hud.Draw(spy, 25, 65, 100, 12, 3.f / 60.f);
    checkHealth(spy, 75, 1.f + (.5f-1.f)*(1.f/60.f)/.25f); // cap change preserves in-flight fill
    spy.clear();
    hud.Draw(spy, 25, 65, 100, 12, 4.f / 60.f);
    checkHealth(spy, 75.2f, 1.f + (25.f/65.f-1.f)*(2.f/60.f)/.25f);
    std::cout << "HUD health length/cap/fill cadence and focus/orb spy PASS\n";
}
