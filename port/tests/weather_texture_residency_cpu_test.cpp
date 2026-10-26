#include "../game/WeatherTextureResidency.h"
#include <cassert>
#include <string>
#include <vector>
using namespace deathleak;
struct SpyRenderer final : render::Renderer {
    int loads = 0;
    int draws = 0;
    bool failNext = false;
    std::vector<std::string> paths;
    bool Initialize(const char*, int, int, bool, std::string&) override { return true; }
    render::TextureId LoadTexture(const std::string& path, std::string& error) override {
        ++loads; paths.push_back(path);
        if (failNext) { failNext = false; error = "injected"; return 0; }
        return static_cast<render::TextureId>(loads);
    }
    void BeginFrame(render::Color) override {}
    void DrawSprite(const render::Sprite&) override { ++draws; }
    void FillRect(render::Rect, render::Color) override {}
    void ApplyScenePostprocess() override {}
    void EndFrame() override {}
    bool PollQuit() override { return false; }
    bool KeyDown(int) const override { return false; }
    render::MouseState Mouse() const override { return {}; }
    bool SaveFrame(const std::string&, std::string&) override { return true; }
};
int main() {
    SpyRenderer renderer;
    game::WeatherRain rain;
    game::WeatherTextureResidency residency;
    residency.Prepare(renderer, rain, false, false);
    assert(renderer.loads == 0);
    residency.Prepare(renderer, rain, true, false);
    assert(renderer.loads == 1 && residency.Rain() == 1);
    assert(renderer.paths[0] == "Game/Media/rain_drop.png");
    residency.Prepare(renderer, rain, true, true);
    assert(renderer.loads == 2 && residency.Lightning() == 2);
    assert(renderer.paths[1] == "Game/Media/lightning_layer.png");
    for (int i = 0; i < 90; ++i) rain.Update(1.f / 60.f, true, 0.f, 0.f);
    const game::WeatherTextureResidency& readOnly = residency;
    for (int i = 0; i < 100000; ++i) {
        assert(readOnly.Rain() == 1 && readOnly.Lightning() == 2);
        rain.Draw(renderer, 0, 0);
    }
    assert(renderer.loads == 2 && renderer.draws > 0);
    residency.Prepare(renderer, rain, true, true);
    assert(renderer.loads == 2);
    SpyRenderer retryRenderer;
    game::WeatherRain retryRain;
    game::WeatherTextureResidency retry;
    retryRenderer.failNext = true;
    retry.Prepare(retryRenderer, retryRain, true, false);
    assert(retry.Rain() == 0 && retryRenderer.loads == 1);
    retry.Prepare(retryRenderer, retryRain, true, false);
    assert(retry.Rain() == 2 && retryRenderer.loads == 2);
}
