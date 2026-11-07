#include "../game/ParticleSpray.h"
#include <cassert>
#include <cmath>
#include <fstream>
#include <iterator>
#include <regex>
#include <string>
#include <vector>

using namespace deathleak;

struct SpyRenderer final : render::Renderer {
    std::vector<render::Sprite> submitted;
    bool Initialize(const char*, int, int, std::string&) override { return true; }
    render::TextureId LoadTexture(const std::string&, std::string&) override { return 1; }
    void BeginFrame(render::Color) override {}
    void DrawSprite(const render::Sprite& sprite) override { submitted.push_back(sprite); }
    void FillRect(render::Rect, render::Color) override {}
    void ApplyScenePostprocess() override {}
    void EndFrame() override {}
    bool PollQuit() override { return false; }
    bool KeyDown(int) const override { return false; }
    render::MouseState Mouse() const override { return {}; }
    bool SaveFrame(const std::string&, std::string&) override { return false; }
};

int main(int argc, char** argv) {
    assert(argc == 2);
    std::ifstream input(argv[1]);
    assert(input.good());
    const std::string source((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    const auto begin = source.find("void LevelScene::EmitIntroSwordPullBurst(");
    const auto end = source.find("void LevelScene::SetIntroBodyPhase(", begin);
    assert(begin != std::string::npos && end != std::string::npos);
    const auto emitter = source.substr(begin, end - begin);
    assert(emitter.find("for (int n = 0; n < 2; ++n)") != std::string::npos);
    const std::regex fadeAssignment(R"(spray\.fadeIn\s*=\s*([0-9.]+)f;\s*spray\.fadeOut\s*=\s*([0-9.]+)f;)");
    std::smatch match;
    assert(std::regex_search(emitter, match, fadeAssignment));
    assert(std::distance(std::sregex_iterator(emitter.begin(), emitter.end(), fadeAssignment),
                         std::sregex_iterator()) == 1);
    game::ParticleSpray spray;
    spray.fadeIn = std::stof(match[1]);
    spray.fadeOut = std::stof(match[2]);
    assert(std::abs(spray.fadeIn - 5.f) < 1e-6f);
    assert(std::abs(spray.fadeOut - 1.f) < 1e-6f);
    spray.texture = 17;
    spray.looping = false;
    spray.warmed = true;
    game::ParticleSpray::Particle particle;
    particle.lifetime = .75f;
    particle.size = particle.startSize = 90.f;
    particle.brightness = 1.f;
    spray.particles.push_back(particle);
    SpyRenderer spy;

    spray.Update(.15f); // 20% of lifetime: source 5.0 fade-in yields 0.04 alpha.
    spray.Draw(spy, 0, 0);
    assert(spy.submitted.size() == 1);
    assert(std::abs(spray.particles.front().alpha - .04f) < 1e-6f);
    assert(spy.submitted.back().tint.a == 10);
    assert(spy.submitted.back().flipX && !spy.submitted.back().flipY);

    spy.submitted.clear();
    spray.Update(.525f); // 90% of lifetime: fade-out branch never fires; fade-in gives 0.18.
    spray.Draw(spy, 0, 0);
    assert(spy.submitted.size() == 1);
    assert(std::abs(spray.particles.front().alpha - .18f) < 1e-5f);
    assert(spy.submitted.back().tint.a == 46);
}
