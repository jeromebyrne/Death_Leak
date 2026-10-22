# Player falling-frame transition fix — 2026-10-22

Scope: source-backed software change only; no GPU, display, gameplay capture, or broad integration. Files changed: `port/game/LevelScene.cpp` and `.h`. Existing unrelated worktree edits were untouched.

## Implementation

- `LevelScene` maintains an off-solid duration and stamps the first dry, non-wall descending visual update. This is equivalent to passing the already-post-motion `PlayerMotion::airTime_` through the caller, without changing `main.cpp` (outside this slice). The state resets on landing/re-ascent and level load.
- Uses the Windows `-0.5` units/tick upward tolerance (`-30` units/s in the port), and retains the existing body sequence, frame, and fractional frame clock until descent time is **strictly greater** than 0.05 s or off-solid time is **strictly greater** than 0.25 s.
- Melee, roll, water, dash, and wall slide retain their selection precedence; the hold does not intercept dash's existing `JumpingDown` stand-in. Arm update remains active, using the still-selected body sequence while held.

## Deterministic verification

From repository root, the isolated CPU-only harness below was compiled and passed:

```sh
g++ -O0 -std=c++17 -w -Iport/compat /tmp/deathleak_player_fall_test.cpp port/game/LevelScene.cpp Game/tinyxml/tinyxml.cpp Game/tinyxml/tinyxmlerror.cpp Game/tinyxml/tinyxmlparser.cpp Game/tinyxml/tinystr.cpp -o /tmp/deathleak_player_fall_test
/tmp/deathleak_player_fall_test
# PASS fall delay/long airtime/reset/priority
```

It checks a short-apex held frame through the strict threshold, unchanged fractional clock, long-airtime immediate transition, landing/re-ascent reset, and dash/wall/water/roll/melee precedence. Existing `port/tests/player_arm_parity_test.cpp` independently rebuilt and passed. `git diff --check` passed. Full Linux executable compilation was attempted outside the shared build directory but blocked by absent `sdl2`/`libpng` pkg-config metadata and `SDL.h` in this sandbox; no runtime or visual parity claim is made.

## Reproducible harness

Save this block to `/tmp/deathleak_player_fall_test.cpp` and run the command above:

```cpp
// Source-authored Player ID2 arm composition, finite timing, and draw order.
#include "../game/LevelScene.h"
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

class CaptureRenderer final : public deathleak::render::Renderer {
public:
    std::unordered_map<std::string, deathleak::render::TextureId> textures;
    std::vector<deathleak::render::Sprite> drawn;
    bool Initialize(const char*, int, int, std::string&) override { return true; }
    deathleak::render::TextureId LoadTexture(const std::string& path, std::string&) override {
        auto [it, inserted] = textures.try_emplace(path, static_cast<deathleak::render::TextureId>(textures.size() + 1));
        return it->second;
    }
    void BeginFrame(deathleak::render::Color) override {}
    void DrawSprite(const deathleak::render::Sprite& sprite) override { drawn.push_back(sprite); }
    void FillRect(deathleak::render::Rect, deathleak::render::Color) override {}
    void ApplyScenePostprocess() override {}
    void EndFrame() override {}
    bool PollQuit() override { return false; }
    bool KeyDown(int) const override { return false; }
    deathleak::render::MouseState Mouse() const override { return {}; }
    bool SaveFrame(const std::string&, std::string&) override { return false; }
};

int main() {
    setenv("DEATHLEAK_RENDER_ONLY_XML_IDS", "2", 1);
    CaptureRenderer renderer;
    deathleak::game::LevelScene scene;
    std::string error;
    assert(scene.Load("Game/XmlFiles/levels/forest_exploration_1.xml", renderer, error));
    const auto x = scene.PlayerX(), y = scene.PlayerY();
    auto body = [&]() {
        renderer.drawn.clear(); scene.Draw(renderer, x, y, x, y);
        assert(!renderer.drawn.empty());
        return renderer.drawn.back().texture;
    };
    auto id = [&](const char* path) { return renderer.textures.at(std::string("Game/") + path); };
    auto tick = [&](float dt, float vy, bool ground=false, int wall=0, bool roll=false, bool dash=false, bool water=false, float melee=0) {
        scene.UpdatePlayerVisual(dt, 0, vy, ground, wall, roll, dash, water, melee, 1, false);
    };
    constexpr auto up0 = "Media/characters/player/jump_up_default/jump_up_default_000.png";
    constexpr auto up1 = "Media/characters/player/jump_up_default/jump_up_default_001.png";
    constexpr auto down0 = "Media/characters/player/jump_down_default/jump_down_default_000.png";
    tick(.01f, 100); assert(body() == id(up0));
    tick(.01f, 100); assert(body() == id(up1));
    auto held = body();
    for (int i=0;i<7;++i) { tick(.01f, -100); if (i<6) assert(body() == held); }
    assert(body() == id(down0)); // strict threshold: strictly after 0.05 s releases
    tick(.01f, 0, true);
    tick(.01f, 100); assert(body() == id(up0));
    tick(.01f, -100); assert(body() == id(up0)); // landing resets descent edge
    tick(.01f, 100); assert(body() == id(up1)); // fractional clock was held
    for (int i=0;i<25;++i) tick(.01f, 100);
    tick(.01f, -100); assert(body() == id(down0)); // off-solid > 0.25 bypass
    tick(.01f, 0, true);
    tick(.01f, -100, false, 0, false, true); assert(body() == id(down0)); // dash
    tick(.01f, -100, false, 1); assert(body() != id(down0)); // wall slide
    tick(.01f, -100, false, 0, false, false, true); assert(body() != id(down0)); // water
    tick(.01f, -100, false, 0, true); assert(body() != id(down0)); // roll
    tick(.01f, -100, false, 0, false, false, false, .02f); assert(body() != id(down0)); // melee
    std::cout << "PASS fall delay/long airtime/reset/priority\n";
}
```

## Remaining caveat

Windows stamps the descent edge before its animation update with per-tick velocity, while the Linux caller supplies post-motion units/s. The `-30` units/s threshold is source-equivalent in magnitude, but exact transition ticks can still differ because of update order and physics integration. Visual confirmation belongs to the separately controlled GPU lane.
