#pragma once
#include "PresentationCommandPacket.h"
#include "RefreshSlotScheduler.h"

namespace deathleak::linux_port {
// A display-only consumer of the last sealed, renderer-owned frame. This class
// has no gameplay, input, audio, capture, film or shader-clock dependency.
class RefreshPresentationService {
public:
    using Clock = RefreshSlotScheduler::Clock;
    enum class Result { NotDue, NoSnapshot, Presented, SurfaceChanged };
    RefreshPresentationService(Clock::time_point origin, int hz,
                               PresentationRecordingRenderer& recorder)
        : slots_(origin, hz), recorder_(recorder) {}
    Result TryPresent(Clock::time_point slotTime) {
        if (!slots_.Due(slotTime)) return Result::NotDue;
        if (!recorder_.Packet().Sealed()) return Result::NoSnapshot;
        if (!recorder_.PresentationSurfaceStable()) return Result::SurfaceChanged;
        recorder_.Replay();
        if (!recorder_.PresentationSurfaceStable()) return Result::SurfaceChanged;
        recorder_.PresentReplay();
        slots_.Presented(Clock::now());
        ++repeatPresents_;
        return Result::Presented;
    }
    void OrdinaryPresented(Clock::time_point when) { slots_.Presented(when); }
    unsigned long long RepeatPresents() const { return repeatPresents_; }
private:
    RefreshSlotScheduler slots_;
    PresentationRecordingRenderer& recorder_;
    unsigned long long repeatPresents_ = 0;
};
} // namespace deathleak::linux_port
