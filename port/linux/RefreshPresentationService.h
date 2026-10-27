#pragma once
#include "PresentationCommandPacket.h"
#include "FreshPresentationPacket.h"
#include <algorithm>
#include <functional>
#include "RefreshSlotScheduler.h"

namespace deathleak::linux_port {
// A display-only consumer of the last sealed, renderer-owned frame. This class
// has no gameplay, input, audio, capture, film or shader-clock dependency.
class RefreshPresentationService {
public:
    using Clock = RefreshSlotScheduler::Clock;
    enum class Result { NotDue, NoSnapshot, Presented, SurfaceChanged, ObserverFailed };
    RefreshPresentationService(Clock::time_point origin, int hz,
                               PresentationRecordingRenderer& recorder, bool syntheticSlots = false)
        : slots_(origin, hz), recorder_(recorder), syntheticSlots_(syntheticSlots) {}
    using ExtraSlotObserver = std::function<bool(double, const FreshPresentationPacket::SampledFrame&)>;
    void SetExtraSlotObserver(ExtraSlotObserver observer) { extraSlotObserver_ = std::move(observer); }
    Result TryPresent(Clock::time_point slotTime) {
        if (!slots_.Due(slotTime)) return Result::NotDue;
        if (!recorder_.Packet().Sealed()) return Result::NoSnapshot;
        if (!recorder_.PresentationSurfaceStable()) return Result::SurfaceChanged;
        std::size_t sampledCommands = 0;
        if (fresh_) {
            const double alpha = std::clamp(std::chrono::duration<double>(slotTime - fixedBoundary_).count() * 60.0, 0.0, 1.0);
            const auto sample = fresh_->Sample(alpha);
            recorder_.ReplayPacket(sample.packet);
            sampledCommands = sample.sampledCommands;
            if (extraSlotObserver_ && !extraSlotObserver_(alpha, sample)) return Result::ObserverFailed;
        } else recorder_.Replay();
        if (!recorder_.PresentationSurfaceStable()) return Result::SurfaceChanged;
        recorder_.PresentReplay();
        slots_.Presented(syntheticSlots_ ? slotTime : Clock::now());
        ++repeatPresents_;
        if (sampledCommands) ++freshPresents_;
        return Result::Presented;
    }
    void OrdinaryPresented(Clock::time_point when) { slots_.Presented(when); }
    // Publishing a completed fixed frame does not itself consume a display slot.
    // The live display loop asks TryPresent only when the independent slot is due.
    void SealFreshBoundary(const FreshPresentationPacket& fresh, Clock::time_point when) {
        fresh_ = &fresh;
        fixedBoundary_ = when;
    }
    double UntilNextSeconds(Clock::time_point now) const { return slots_.UntilNextSeconds(now); }
    // The update has rendered and sealed N. Present N-1 at alpha=0, then
    // monotonically advance toward N on display-only slots. No rewind after
    // an ordinary alpha=1 present, and scene cuts fail closed to N.
    Result PresentFreshBoundary(const FreshPresentationPacket& fresh, Clock::time_point when) {
        if (!recorder_.PresentationSurfaceStable()) return Result::SurfaceChanged;
        SealFreshBoundary(fresh, when);
        const auto sample = fresh.Sample(0.0);
        recorder_.ReplayPacket(sample.packet);
        if (!recorder_.PresentationSurfaceStable()) return Result::SurfaceChanged;
        recorder_.PresentReplay();
        slots_.Presented(syntheticSlots_ ? when : Clock::now());
        return Result::Presented;
    }
    unsigned long long FreshPresents() const { return freshPresents_; }
    unsigned long long RepeatPresents() const { return repeatPresents_; }
private:
    RefreshSlotScheduler slots_;
    PresentationRecordingRenderer& recorder_;
    unsigned long long repeatPresents_ = 0;
    unsigned long long freshPresents_ = 0;
    const FreshPresentationPacket* fresh_ = nullptr;
    Clock::time_point fixedBoundary_{};
    bool syntheticSlots_ = false;
    ExtraSlotObserver extraSlotObserver_;
};
} // namespace deathleak::linux_port
