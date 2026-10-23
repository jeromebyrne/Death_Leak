#include "../game/Combat.h"
#include <cassert>
#include <cmath>
#include <iostream>

class CaptureRenderer final : public deathleak::render::Renderer {
public:
    bool Initialize(const char*, int, int, std::string&) override { return true; }
    deathleak::render::TextureId LoadTexture(const std::string&, std::string&) override { return 1; }
    void BeginFrame(deathleak::render::Color) override {}
    void DrawSprite(const deathleak::render::Sprite&) override {}
    void FillRect(deathleak::render::Rect, deathleak::render::Color) override {}
    void ApplyScenePostprocess() override {}
    void EndFrame() override {}
    bool PollQuit() override { return false; }
    bool KeyDown(int) const override { return false; }
    deathleak::render::MouseState Mouse() const override { return {}; }
    bool SaveFrame(const std::string&, std::string&) override { return false; }
};

int main() {
    using namespace deathleak::game;
    CaptureRenderer renderer;
    LevelScene scene;
    constexpr float x = 100000.f, y = 100000.f;
    struct Case { PlayerShotLaunchState state; float offset; };
    const Case cases[] = {
        {{false, false, false}, 25.f}, // standing
        {{true,  false, false},  0.f}, // crouching
        {{false, true,  false}, 90.f}, // rising
        {{false, false, true }, 65.f}, // falling
        {{true,  true,  false}, 65.f}, // source applies both offsets
        {{true,  false, true }, 40.f},
        {{false, true,  true }, 90.f}, // up wins the source else-if
        {{true,  true,  true }, 65.f}
    };
    for (const auto& test : cases) {
        for (int diagonal = 0; diagonal < 2; ++diagonal) {
            for (int direction : {-1, 1}) {
                Combat combat;
                const float aimX = static_cast<float>(direction);
                const float aimY = diagonal ? 1.f : 0.f;
                combat.Update(scene, x, y, 0.f, true, false, false, false,
                              direction, renderer, aimX, aimY, test.state);
                assert(combat.Fired() == 1 && combat.ShotCount() == 1);
                const Shot& shot = combat.Shots().front();
                assert(shot.playerKnife);
                assert(shot.x == x + direction * 10.f);
                assert(shot.y == y + test.offset);
                const float unit = diagonal ? 1.f / std::sqrt(2.f) : 1.f;
                assert(std::abs(shot.vx - direction * unit * 1200.f) < .001f);
                assert(std::abs(shot.vy - aimY * unit * 1200.f) < .001f);
                assert(shot.launchDirectionX * direction > 0.f);
            }
        }
    }
    std::cout << "PASS: player shot launch state, branch order, and horizontal/diagonal aim\n";
}
