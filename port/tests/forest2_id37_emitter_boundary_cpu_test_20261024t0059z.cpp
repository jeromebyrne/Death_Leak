// Forest-2's ID37 is an authored leaf emitter, not Forest-1's ID37 layer.
// Keep this diagnostic's private-member access local: it fixes particle states
// after XML load so the draw gate can be tested independently of random warm-up.
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "../render/Renderer.h"
#include "../audio/SoundPlayer.h"
#include "../game/ParticleSpray.h"
#include "../game/ShaderClock.h"
#define private public
#include "../game/LevelScene.h"
#undef private

using deathleak::game::ParticleSpray;
using deathleak::render::Sprite;

struct Forest2Id37Spy final : deathleak::render::Renderer {
    std::vector<std::string> loaded;
    std::vector<Sprite> drawn;
    bool Initialize(const char*, int, int, std::string&) override { return true; }
    deathleak::render::TextureId LoadTexture(const std::string& path, std::string&) override {
        loaded.push_back(path);
        return static_cast<deathleak::render::TextureId>(loaded.size());
    }
    void BeginFrame(deathleak::render::Color) override {}
    void DrawSprite(const Sprite& sprite) override { drawn.push_back(sprite); }
    void FillRect(deathleak::render::Rect, deathleak::render::Color) override {}
    void ApplyScenePostprocess() override {}
    void EndFrame() override {}
    bool PollQuit() override { return false; }
    bool KeyDown(int) const override { return false; }
    deathleak::render::MouseState Mouse() const override { return {}; }
    bool SaveFrame(const std::string&, std::string&) override { return false; }
};

// Independent transcription of Windows Camera2D::IsObjectInView: origin first,
// then strict emitter bounds overlap in scaled 1280x720 camera space.
static bool SourceAdmits(const ParticleSpray& spray, float cameraX, float cameraY) {
    constexpr float scale = 2.f / 3.f;
    const float left = cameraX*scale - 640.f, right = cameraX*scale + 640.f;
    const float bottom = cameraY*scale - 360.f, top = cameraY*scale + 360.f;
    const float x = spray.x*scale, y = spray.y*scale;
    if (x > left && x < right && y > bottom && y < top) return true;
    return x + spray.width*scale*.5f > left && x - spray.width*scale*.5f < right &&
           y + spray.height*scale*.5f > bottom && y - spray.height*scale*.5f < top;
}

int main() {
    // The filter suppresses unrelated XML draw commands, not loading. Test
    // texture admission and the authored spray object independently.
    setenv("DEATHLEAK_RENDER_ONLY_XML_IDS", "37", 1);
    unsetenv("DEATHLEAK_RENDER_HIDE_XML_IDS");
    Forest2Id37Spy spy;
    deathleak::game::LevelScene scene;
    std::string error;
    assert(scene.Load("Game/XmlFiles/levels/forest_exploration_2.xml", spy, error));
    const std::string leafAsset = "Game/Media/levels/forest_exploration_1/oak_leaf.png";
    assert(std::count(spy.loaded.begin(), spy.loaded.end(), leafAsset) == 1 &&
           "Forest-2 ID37 must pass the loader allowlist and load oak_leaf.png");
    assert(scene.particleSprays_.size() == 1);
    auto& spray = scene.particleSprays_.front();
    assert(spray.xmlId == "37" && spray.depth == 1750);
    assert(std::abs(spray.x - (-20.714647f)) < .001f);
    assert(std::abs(spray.y - 4345.097168f) < .001f);
    assert(std::abs(spray.width - 50000.269531f) < .01f);
    assert(std::abs(spray.height - 2000.207764f) < .01f);
    assert(spray.particles.size() == 4);
    const float expectedLifetimes[] = {40.f, 31.5f, 23.f, 14.5f};
    for (size_t i = 0; i < spray.particles.size(); ++i)
        assert(std::abs(spray.particles[i].lifetime - expectedLifetimes[i]) < .001f);

    // One warm leaf remains in-frame just below the *emitter* visibility
    // boundary. Three other leaves are well above this camera's viewport.
    spray.warmed = true;
    for (size_t i = 0; i < spray.particles.size(); ++i) {
        auto& p = spray.particles[i];
        p.startX = p.x = p.drawX = spray.x;
        p.startY = spray.y;
        p.y = p.drawY = i == 0 ? 2703.f : spray.y;
        p.size = p.startSize = p.drawSize = 80.f;
        p.speed = p.startSpeed = 4.f;
        p.directionX = 0.f; p.directionY = -1.f;
        p.brightness = 1.f; p.alpha = 1.f;
        p.elapsed = i == 0 ? 6.8f : 0.f;
        p.dead = false;
    }
    const float boundary = spray.y - spray.height*.5f - 540.f;
    const float offY = boundary - 1.f, onY = boundary + 1.f;
    assert(!SourceAdmits(spray, spray.x, offY));
    assert(SourceAdmits(spray, spray.x, onY));
    assert(!SourceAdmits(spray, spray.x, boundary));

    // Direct ParticleSpray::Draw clips individual leaves only: this is the
    // negative control proving a visible particle exists off-emitter.
    spray.Draw(spy, spray.x, offY);
    assert(spy.drawn.size() == 1);
    spy.drawn.clear();
    scene.Draw(spy, spray.x, offY, spray.x, offY);
    assert(spy.drawn.empty() && "off-camera emitter must suppress visible leaf");
    scene.Draw(spy, spray.x, boundary, spray.x, boundary);
    assert(spy.drawn.empty() && "strict emitter-boundary contact must reject");
    scene.Draw(spy, spray.x, onY, spray.x, onY);
    assert(spy.drawn.size() == 1 && "in-view emitter must submit one visible leaf");
    const auto& leaf = spy.drawn.front();
    assert(leaf.texture == spray.texture && leaf.flipX);
    assert(std::abs(leaf.destination.width - 80.f*2.f/3.f) < .001f);
    assert(std::abs(leaf.destination.height - 80.f*2.f/3.f) < .001f);
    std::cout << "PASS: Forest-2 ID37 authored admission and emitter-boundary draw gate\n";
}
