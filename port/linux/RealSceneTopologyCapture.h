#pragma once
#include "InterpolationTopologyOracle.h"
#include "../game/LevelScene.h"
#include <memory>
#include <utility>

namespace deathleak::linux_port {
// Diagnostic command snapshot for ordinary authored sprites only. The callback
// is invoked after the real DrawSprite submission, so omitted/cut objects are
// absent rather than inferred from XML. No rendering behavior is changed.
struct EmittedSceneCommand {
    TopologyKey key;
    int painterOrdinal = 0; // observed order within this admitted family
    render::Sprite command;
};
struct RealSceneSnapshot {
    TopologySnapshot topology;
    std::vector<EmittedSceneCommand> commands;
};
inline float OrdinaryCameraFactorX(int depth) {
    switch (depth) {
    case 5999: return .03f; case 6000: return .25f; case 5500: return .50f;
    case 5000: return .75f; case 1500: return 1.10f; case 1000: return 1.30f;
    case 500: case 20: return 1.80f; default: return 1.f;
    }
}
inline float OrdinaryCameraFactorY(int depth) {
    switch (depth) {
    case 5999: return .90f; case 6000: return .95f; case 5500: return .97f;
    case 5000: return .99f; case 1500: return 1.05f; case 1000: return 1.08f;
    case 500: case 20: return 1.20f; default: return 1.f;
    }
}
class RealSceneCapture {
public:
    RealSceneCapture(std::uint64_t epoch, float cameraX, float cameraY, bool cut = false) {
        snapshot_.topology.sceneEpoch = epoch;
        snapshot_.topology.cameraX = cameraX * (2.f / 3.f);
        snapshot_.topology.cameraY = -cameraY * (2.f / 3.f);
        snapshot_.topology.discontinuity = cut;
    }
    void Observe(const game::SceneSprite& source, const render::Sprite& command) {
        const float fx = OrdinaryCameraFactorX(source.depth);
        const float fy = OrdinaryCameraFactorY(source.depth);
        const int ordinal = static_cast<int>(snapshot_.commands.size());
        snapshot_.commands.push_back({{source.stableIdentity, 0}, ordinal, command});
        snapshot_.topology.items.push_back({{source.stableIdentity, 0}, ordinal, true,
            command.destination.x + snapshot_.topology.cameraX * fx,
            command.destination.y + snapshot_.topology.cameraY * fy, fx, fy});
    }
    RealSceneSnapshot Seal() && { return std::move(snapshot_); }
private:
    RealSceneSnapshot snapshot_;
};
// The same identity schema accepts actual emitted particle commands. A spray
// owns <=1024 slots; reserve 16 bits per slot so different emitters cannot
// alias when a vector slot is recycled or a new spray is born.
inline TopologyKey EmittedParticleKey(const game::ParticleSpray& spray,
                                      const game::ParticleSpray::Particle& particle) {
    return {(spray.stableIdentity << 16) | particle.stableIdentity, particle.generation};
}
class RealParticleCapture {
public:
    RealParticleCapture(std::uint64_t epoch, float cameraX, float cameraY) {
        snapshot_.topology.sceneEpoch = epoch;
        snapshot_.topology.cameraX = cameraX * (2.f / 3.f);
        snapshot_.topology.cameraY = -cameraY * (2.f / 3.f);
    }
    void Observe(const game::ParticleSpray& spray, const game::ParticleSpray::Particle& particle,
                 const render::Sprite& command) {
        const auto key = EmittedParticleKey(spray, particle);
        const int ordinal = static_cast<int>(snapshot_.commands.size());
        snapshot_.commands.push_back({key, ordinal, command});
        snapshot_.topology.items.push_back({key, ordinal, true,
            command.destination.x + snapshot_.topology.cameraX,
            command.destination.y + snapshot_.topology.cameraY, 1.f, 1.f});
    }
    RealSceneSnapshot Seal() && { return std::move(snapshot_); }
private:
    RealSceneSnapshot snapshot_;
};
struct ImmutableScenePair {
    std::shared_ptr<const RealSceneSnapshot> previous, current;
    explicit ImmutableScenePair(RealSceneSnapshot a, RealSceneSnapshot b)
        : previous(std::make_shared<const RealSceneSnapshot>(std::move(a))),
          current(std::make_shared<const RealSceneSnapshot>(std::move(b))) {}
    TopologyDecision Admission(float alpha) const {
        if (previous->commands.size() != current->commands.size())
            return TopologyDecision::HoldCurrent;
        // The oracle models only position and camera. Any other command change
        // means this family cannot interpolate from these snapshots.
        for (std::size_t i = 0; i < current->commands.size(); ++i) {
            const auto& a = previous->commands[i].command;
            const auto& b = current->commands[i].command;
            if (a.texture != b.texture || a.source.x != b.source.x || a.source.y != b.source.y ||
                a.source.width != b.source.width || a.source.height != b.source.height ||
                a.destination.width != b.destination.width || a.destination.height != b.destination.height ||
                a.tint.r != b.tint.r || a.tint.g != b.tint.g || a.tint.b != b.tint.b || a.tint.a != b.tint.a ||
                a.opacity != b.opacity || a.rotationDegrees != b.rotationDegrees ||
                a.flipX != b.flipX || a.flipY != b.flipY || a.repeatX != b.repeatX ||
                a.uvShiftX != b.uvShiftX || a.uvShiftY != b.uvShiftY || a.rgbGain != b.rgbGain)
                return TopologyDecision::HoldCurrent;
        }
        return SampleTopology(previous->topology, current->topology, alpha).decision;
    }
};
} // namespace deathleak::linux_port
