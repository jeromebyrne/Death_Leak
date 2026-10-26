#include "../linux/FixedStepPacer.h"
#include "../game/ShaderClock.h"
#include <cassert>
#include <chrono>
#include <cmath>
#include <iostream>

using deathleak::linux_port::FixedStepPacer;
using deathleak::game::ShaderClock;

int main() {
    using Clock = FixedStepPacer::Clock;
    constexpr int rates[] = {30, 60, 75, 120, 144, 165, 240};
    for (int hz : rates) {
        const auto start = Clock::time_point{};
        FixedStepPacer pacer(start);
        ShaderClock shader;
        int updates = 0;
        int repeatedSlots = 0;
        double preceding = -1.0;
        for (int slot = 0; slot <= hz * 10; ++slot) {
            const auto at = start + std::chrono::duration_cast<Clock::duration>(
                std::chrono::duration<double>(double(slot) / hz));
            pacer.Accumulate(at);
            if (!pacer.Due()) ++repeatedSlots;
            while (pacer.Due()) {
                pacer.Consume();
                shader.Update(float(FixedStepPacer::StepSeconds));
                shader.Draw();
                shader.RenderCompleted(float(FixedStepPacer::StepSeconds));
                ++updates;
            }
            const double alpha = pacer.PresentationAlpha();
            assert(alpha >= 0.0 && alpha <= 1.0);
            const auto before = shader.PresentationSnapshot();
            const auto again = shader.PresentationSnapshot();
            assert(before.totalTime == again.totalTime);
            assert(before.pixelWobbleTime == again.pixelWobbleTime);
            if (preceding >= 0.0 && slot > 0 && hz == 240 && slot % 4 != 0)
                assert(alpha >= preceding - 1e-5 || alpha < 1e-5);
            preceding = alpha;
        }
        // Bootstrap update at t=0 plus 600 real-time fixed quanta.
        assert(updates == 601);
        assert(pacer.DroppedSteps() == 0);
        assert((repeatedSlots > 0) == (hz > 60));
        assert(std::fabs(shader.PresentationSnapshot().totalTime - 601.f / 60.f) < 0.001f);
        std::cout << hz << "Hz slots=" << hz * 10 + 1 << " updates=" << updates
                  << " display-only=" << repeatedSlots << '\n';
    }
    FixedStepPacer startup(Clock::time_point{});
    startup.Consume();
    startup.RebaseAfterFirstPresent(Clock::time_point{} + std::chrono::seconds(3));
    assert(startup.DroppedSteps() == 0);
    assert(!startup.Due());
    startup.Accumulate(Clock::time_point{} + std::chrono::seconds(3) + std::chrono::milliseconds(17));
    assert(startup.Due());
    assert(startup.DroppedSteps() == 0);
    // A later suspended window cannot request an unbounded catch-up.
    const auto start = Clock::time_point{};
    FixedStepPacer overloaded(start);
    overloaded.Accumulate(start + std::chrono::seconds(3));
    int catchUp = 0;
    while (overloaded.Due()) { overloaded.Consume(); ++catchUp; }
    assert(catchUp == FixedStepPacer::MaxCatchUpSteps);
    assert(overloaded.DroppedSteps() == 61); // 180 elapsed + bootstrap - 120
    assert(overloaded.PresentationAlpha() >= 0.0);
    assert(overloaded.PresentationAlpha() <= 1.0);
    overloaded.Accumulate(start + std::chrono::seconds(2)); // reversed sample ignored
    assert(overloaded.DroppedSteps() == 61);
    overloaded.Accumulate(start + std::chrono::seconds(3) + std::chrono::milliseconds(17));
    assert(overloaded.Due());
    assert(overloaded.DroppedSteps() == 61);
    std::cout << "overload catch-up=" << catchUp << " dropped=" << overloaded.DroppedSteps() << '\n';

}
