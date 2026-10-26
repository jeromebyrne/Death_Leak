#include "../game/FilmFrameResidency.h"
#include <cassert>
#include <string>
#include <vector>
using namespace deathleak;
struct SpyRenderer final : render::Renderer {
    int loads = 0, draws = 0;
    bool failFirst = false;
    std::vector<std::string> paths;
    bool Initialize(const char*, int, int, bool, std::string&) override { return true; }
    render::TextureId LoadTexture(const std::string& path, std::string& error) override {
        ++loads; paths.push_back(path);
        if (failFirst && loads == 1) { error = "injected"; return 0; }
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
    std::string error;
    game::FilmFrameResidency film;
    assert(film.Size() == 4 && film.Frame(0) == 0);
    film.Prepare(renderer, error);
    assert(renderer.loads == 4);
    assert(renderer.paths == std::vector<std::string>({
        "Game/Media/UI/film_frame.png", "Game/Media/UI/film_frame_2.png",
        "Game/Media/UI/film_frame_3.png", "Game/Media/UI/film_frame_4.png"}));
    const game::FilmFrameResidency& readOnly = film;
    for (int i = 0; i < 100000; ++i) {
        const auto id = readOnly.Frame(static_cast<std::size_t>(i) % readOnly.Size());
        assert(id == static_cast<render::TextureId>(i % 4 + 1));
        renderer.DrawSprite({id, {}, {0, 0, 1280, 720}, {255, 255, 255, 51}});
    }
    assert(renderer.loads == 4 && renderer.draws == 100000);
    film.Prepare(renderer, error);
    assert(renderer.loads == 4);
    SpyRenderer retryRenderer;
    retryRenderer.failFirst = true;
    game::FilmFrameResidency retry;
    retry.Prepare(retryRenderer, error);
    assert(retryRenderer.loads == 4 && retry.Frame(0) == 0);
    retry.Prepare(retryRenderer, error);
    assert(retryRenderer.loads == 8 && retry.Frame(0) == 5);
    retry.Prepare(retryRenderer, error);
    assert(retryRenderer.loads == 8);
}
