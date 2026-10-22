#include "../game/LevelScene.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

class CaptureRenderer final : public deathleak::render::Renderer {
public:
    std::vector<int> order;
    bool Initialize(const char*, int, int, std::string&) override { return true; }
    deathleak::render::TextureId LoadTexture(const std::string&, std::string&) override { return 1; }
    void BeginFrame(deathleak::render::Color) override {}
    void DrawSprite(const deathleak::render::Sprite& sprite) override {
        const auto width = static_cast<int>(std::lround(sprite.destination.width * 1.5f));
        order.push_back(width);
        // All quads cover screen center, so last submitted opaque quad owns it.
        assert(sprite.destination.x <= 640 && sprite.destination.x + sprite.destination.width >= 640);
        assert(sprite.destination.y <= 360 && sprite.destination.y + sprite.destination.height >= 360);
    }
    void FillRect(deathleak::render::Rect, deathleak::render::Color) override {}
    void ApplyScenePostprocess() override {}
    void EndFrame() override {}
    bool PollQuit() override { return false; }
    bool KeyDown(int) const override { return false; }
    deathleak::render::MouseState Mouse() const override { return {}; }
    bool SaveFrame(const std::string&, std::string&) override { return false; }
};
int main() {
    CaptureRenderer renderer;
    deathleak::game::LevelScene scene;
    std::string error;
    assert(scene.Load("port/tests/fixtures/combat_painter_order.xml", renderer, error));
    std::vector<deathleak::game::PainterItem> items;
    auto add = [&](int depth, int width) {
        items.push_back({depth, [&, width] {
            renderer.DrawSprite({1, {}, {640.f-width/3.f, 360.f-width/3.f,
                                          width*2.f/3.f, width*2.f/3.f}});
        }});
    };
    // Deliberately unsorted insertion exercises the actual merge, including
    // source NPC, orb, shot, and same-depth pickup ordering.
    add(2500, 45); // player knife
    add(3500, 55); // NPC
    add(3000, 105); // spawned health pickup, after XML player
    add(3250, 60); // orb
    add(2510, 50); // ninja star
    scene.Draw(renderer, 0, 0, 0, 0, std::move(items));
    const std::vector<int> expected{120,55,60,110,105,50,45,100,90,80,70};
    assert(renderer.order == expected);
    assert(renderer.order.back() == 70); // Opaque near-front wins knife overlap.
    std::cout << "PASS: interleaved source depths and foreground-over-knife overlap\n";
}
