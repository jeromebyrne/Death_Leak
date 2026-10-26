#include "../game/ShaderClock.h"
#include <cassert>
#include <type_traits>
#include <iostream>
using deathleak::game::ShaderClock;
static_assert(std::is_same_v<decltype(std::declval<const ShaderClock&>().Draw()), deathleak::game::ShaderClockSnapshot>);
int main() {
    ShaderClock clock;
    clock.RenderCompleted(1.f / 60.f);
    for (int update = 0; update < 1800; ++update) {
        clock.Update(1.f / 60.f);
        clock.Prepare();
        const ShaderClock& view = clock;
        const auto first = view.Draw();
        const bool direction = view.Reversing();
        for (int read = 0; read < 1000; ++read) {
            const auto again = view.Draw();
            assert(first.totalTime == again.totalTime);
            assert(first.pixelWobbleTime == again.pixelWobbleTime);
            assert(direction == view.Reversing());
        }
        clock.RenderCompleted(1.f / 60.f);
    }
    assert(clock.Draw().pixelWobbleTime >= 0.f);
    std::cout << "PASS: 1800 prepared updates and 1.8m same-clock read-only draws\n";
}
