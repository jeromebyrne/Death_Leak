#include "../linux/RefreshSlotScheduler.h"
#include "../linux/FixedStepPacer.h"
#include <cassert>
#include <chrono>
#include <iostream>
using namespace deathleak::linux_port;
int main() {
    using Clock = RefreshSlotScheduler::Clock;
    const auto origin = Clock::time_point{};
    for (int hz : {60, 75, 120, 144, 240}) {
        RefreshSlotScheduler slots(origin, hz);
        FixedStepPacer sim(origin);
        int updates = 0, ordinarySlots = 0, repeatSlots = 0;
        for (int ms = 0; ms <= 10000; ++ms) {
            const auto now = origin + std::chrono::milliseconds(ms);
            sim.Accumulate(now);
            int updatesThisTick = 0;
            while (sim.Due()) { sim.Consume(); ++updates; ++updatesThisTick; }
            if (slots.Due(now)) {
                if (updatesThisTick) ++ordinarySlots;
                else if (updates) ++repeatSlots;
                slots.Presented(now);
            }
        }
        assert(updates == 601);
        assert(ordinarySlots > 0);
        if (hz > 60) assert(repeatSlots > 0);
        assert(slots.NextSlot() >= hz * 10);
    }
    RefreshSlotScheduler dropped(origin, 120);
    dropped.Presented(origin + std::chrono::seconds(3));
    assert(dropped.NextSlot() == 361); // no burst of missed presents
    std::cout << "refresh slots independent of 601 fixed updates; extra slots and missed-slot skips verified\n";
}
