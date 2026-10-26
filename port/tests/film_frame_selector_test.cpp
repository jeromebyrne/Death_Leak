#include "../linux/FilmFrameSelector.h"
#include <cassert>
#include <cstdint>
#include <initializer_list>
int main() {
    deathleak::linux_port::FilmFrameSelector gameplay, final;
    std::uint64_t last = 0;
    std::size_t reference = 0;
    // Exact boundary, 60-Hz slots, long stalls, and independent final-scene phase.
    for (std::uint64_t ms : {0ULL, 90ULL, 91ULL, 92ULL, 183ULL, 1000ULL, 1017ULL, 2000ULL}) {
        if (last + 1000 / 11 < ms) { reference = (reference + 1) % 4; last = ms; }
        gameplay.Prepare(ms, 4);
        assert(gameplay.Index() == reference);
        const auto selected = gameplay.Index();
        assert(gameplay.Index() == selected && gameplay.Index() == selected);
    }
    for (std::uint64_t tick = 0; tick < 600; ++tick) {
        const auto ms = tick * 1000 / 60;
        if (last + 1000 / 11 < ms) { reference = (reference + 1) % 4; last = ms; }
        gameplay.Prepare(ms, 4);
        assert(gameplay.Index() == reference);
    }
    assert(final.Index() == 0);
    final.Prepare(1000, 4);
    assert(final.Index() == 1);
}
