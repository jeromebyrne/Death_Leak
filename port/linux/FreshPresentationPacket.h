#pragma once
#include "MixedCommandInterpolationGate.h"
#include "../game/PresentationPose.h"
#include <cmath>
#include <utility>

namespace deathleak::linux_port {
// Read-only full post-HUD packet sampling at a fixed-update boundary. Both
// snapshots are complete painter streams; no game object or clock is consulted.
// A display scheduler may consume this only after choosing a temporal policy.
class FreshPresentationPacket {
public:
    struct SampledFrame {
        PresentationCommandPacket packet;
        game::PresentationPose pose;
        std::size_t sampledCommands = 0;
    };
    void Seal(FullFrameSnapshot frame, const game::PresentationPose& pose) {
        previous_ = std::move(current_);
        current_ = std::move(frame);
        poses_.Seal(pose, previous_.packet.Sealed() &&
            previous_.sceneEpoch == current_.sceneEpoch &&
            !previous_.discontinuity && !current_.discontinuity);
    }
    SampledFrame Sample(double alpha) const {
        SampledFrame out{current_.packet, poses_.Sample(alpha), 0};
        if (!current_.packet.Sealed() || !previous_.packet.Sealed() ||
            !poses_.HasContinuousPair() || !std::isfinite(alpha) ||
            alpha < 0.0 || alpha >= 1.0) return out;
        const auto decision = AdmitMixedCommands(previous_, current_, static_cast<float>(alpha));
        if (!decision.valid || !decision.moving) return out;
        out.packet = SampleMixedCommands(previous_, current_, static_cast<float>(alpha));
        out.sampledCommands = decision.moving;
        return out;
    }
private:
    FullFrameSnapshot previous_{};
    FullFrameSnapshot current_{};
    game::PresentationPoseHistory poses_{};
};
}
