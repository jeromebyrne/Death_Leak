#include "../linux/RealSceneTopologyCapture.h"
#include "../linux/FixedStepPacer.h"
#include <cassert>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <set>
using namespace deathleak;
class SpyRenderer final : public render::Renderer {
public:
    std::vector<render::Sprite> emitted;
    bool Initialize(const char*, int, int, bool, std::string&) override { return true; }
    render::TextureId LoadTexture(const std::string&, std::string&) override { return 1; }
    void BeginFrame(render::Color) override {}
    void DrawSprite(const render::Sprite& s) override { emitted.push_back(s); }
    void FillRect(render::Rect, render::Color) override {}
    void ApplyScenePostprocess() override {}
    void EndFrame() override {}
    bool PollQuit() override { return false; }
    bool KeyDown(int) const override { return false; }
    render::MouseState Mouse() const override { return {}; }
    bool SaveFrame(const std::string&, std::string&) override { return false; }
};
static linux_port::RealSceneSnapshot Capture(game::LevelScene& scene, SpyRenderer& renderer,
                                             float cameraX, float cameraY, bool cut = false) {
    linux_port::RealSceneCapture capture(scene.SceneEpoch(), cameraX, cameraY, cut);
    renderer.emitted.clear();
    scene.Draw(renderer, cameraX, cameraY, scene.PlayerX(), scene.PlayerY(), {}, 1.f, 1.f,
               {}, false, {}, [&](const game::SceneSprite& source, const render::Sprite& command) {
                   assert(!renderer.emitted.empty());
                   const auto& actual = renderer.emitted.back();
                   assert(actual.texture == command.texture);
                   assert(actual.destination.x == command.destination.x);
                   capture.Observe(source, command);
               });
    return std::move(capture).Seal();
}
int main() {
    setenv("DEATHLEAK_RENDER_ONLY_XML_IDS", "1,17,19", 1);
    SpyRenderer renderer;
    game::LevelScene scene;
    std::string error;
    assert(scene.Load("Game/XmlFiles/levels/forest_exploration_1.xml", renderer, error));
    const float x = scene.CameraX(), y = scene.CameraY();
    auto first = Capture(scene, renderer, x, y);
    auto second = Capture(scene, renderer, x + 10.f, y);
    assert(!first.commands.empty());
    assert(first.commands.size() == first.topology.items.size());
    std::set<linux_port::TopologyKey> unique;
    for (size_t i = 0; i < first.commands.size(); ++i) {
        assert(first.commands[i].key.object != 0);
        assert(unique.insert(first.commands[i].key).second);
        assert(first.commands[i].painterOrdinal == int(i));
    }
    linux_port::ImmutableScenePair pair(std::move(first), std::move(second));
    assert(pair.Admission(.5f) == linux_port::TopologyDecision::Interpolate);
    const auto frozenX = pair.current->commands.front().command.destination.x;
    auto repeat = Capture(scene, renderer, x + 10.f, y);
    assert(pair.current->commands.front().command.destination.x == frozenX);
    assert(pair.Admission(.5f) == pair.Admission(.5f));
    assert(repeat.commands.front().key == pair.current->commands.front().key);
    auto far = Capture(scene, renderer, x + 100000.f, y);
    linux_port::ImmutableScenePair visibility(*pair.current, std::move(far));
    assert(visibility.Admission(.5f) == linux_port::TopologyDecision::HoldCurrent);
    auto cameraCut = Capture(scene, renderer, x + 10.f, y, true);
    linux_port::ImmutableScenePair cut(*pair.previous, std::move(cameraCut));
    assert(cut.Admission(.5f) == linux_port::TopologyDecision::HoldCurrent);
    const auto oldEpoch = scene.SceneEpoch();
    assert(scene.Load("Game/XmlFiles/levels/forest_exploration_1.xml", renderer, error));
    assert(scene.SceneEpoch() != oldEpoch);
    auto reloaded = Capture(scene, renderer, x, y);
    linux_port::ImmutableScenePair epoch(*pair.previous, std::move(reloaded));
    assert(epoch.Admission(.5f) == linux_port::TopologyDecision::HoldCurrent);
    game::ParticleSpray spray;
    game::ParticleSpray::Particle particle;
    particle.stableIdentity = 1; particle.lifetime = .01f; particle.drawSize = 10.f;
    spray.texture = 1; spray.stableIdentity = 777;
    spray.particles.push_back(particle);
    auto captureParticles = [&] {
        linux_port::RealParticleCapture capture(scene.SceneEpoch(), 0.f, 0.f);
        spray.Draw(renderer, 0.f, 0.f, [&](const game::ParticleSpray::Particle& p, const render::Sprite& command) {
            assert(renderer.emitted.back().destination.x == command.destination.x);
            capture.Observe(spray, p, command);
        });
        return std::move(capture).Seal();
    };
    auto particleA = captureParticles();
    assert(particleA.commands.size() == 1 && particleA.commands[0].key.generation == 1);
    spray.Update(1.f/60.f);
    auto particleB = captureParticles();
    assert(particleB.commands.size() == 1 && particleB.commands[0].key.generation == 2);
    linux_port::ImmutableScenePair reused(std::move(particleA), std::move(particleB));
    assert(reused.Admission(.5f) == linux_port::TopologyDecision::HoldCurrent);
    spray.looping = false;
    spray.Update(1.f/60.f);
    auto dead = captureParticles();
    assert(dead.commands.empty());
    linux_port::ImmutableScenePair death(*reused.current, std::move(dead));
    assert(death.Admission(.5f) == linux_port::TopologyDecision::HoldCurrent);
    game::ParticleSpray::Particle born = particle; born.stableIdentity = 2;
    spray.particles.push_back(born);
    auto newBirth = captureParticles();
    assert(newBirth.commands.size() == 1 && newBirth.commands[0].key.object != reused.current->commands[0].key.object);
    linux_port::ImmutableScenePair birth(*death.current, std::move(newBirth));
    assert(birth.Admission(.5f) == linux_port::TopologyDecision::HoldCurrent);
    using Clock = linux_port::FixedStepPacer::Clock;
    for (int hz : {60,75,120,144,165,240}) {
        linux_port::FixedStepPacer p(Clock::time_point{});
        int updates=0;
        for (int slot=0; slot<=hz*10; ++slot) {
            p.Accumulate(Clock::time_point{} + std::chrono::duration_cast<Clock::duration>(std::chrono::duration<double>(double(slot)/hz)));
            while (p.Due()) { p.Consume(); ++updates; }
        }
        assert(updates == 601 && p.DroppedSteps() == 0);
    }
    std::cout << "real emitted command identity/order, visibility/cut/epoch, particle generation and cadence passed; family_count=" << pair.current->commands.size() << '\n';
}
