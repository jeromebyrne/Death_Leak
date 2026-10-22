#include "../linux/RepeatX.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

using deathleak::render::RepeatXTile;
using deathleak::render::ForEachRepeatXTile;

int main() {
    constexpr float scale = 2.f / 3.f;
    constexpr float quad = 10000.f * scale;
    constexpr float tile = 2048.f * scale;
    std::vector<RepeatXTile> parts;
    ForEachRepeatXTile(0, quad, tile, 2048, quad, true,
                       [&](RepeatXTile part) { parts.push_back(part); });
    assert(parts.size() == 5);
    assert(std::abs(parts.front().offset - 7952.f * scale) < .01f);
    assert(parts.front().sourceWidth == 2048);
    assert(std::abs(parts.back().offset) < .01f);
    assert(std::abs(parts.back().width - 1808.f * scale) < .01f);
    assert(parts.back().sourceWidth == 1808);
    // SDL flips that leftmost source [0,1808]: the quad's left UV is
    // 1808/2048, not 1 as it was when each left-origin tile was flipped.
    assert(parts.back().sourceWidth != 2048);
    parts.clear();
    ForEachRepeatXTile(0, quad, tile, 2048, quad, false,
                       [&](RepeatXTile part) { parts.push_back(part); });
    assert(parts.front().offset == 0 && parts.front().sourceWidth == 2048);
    assert(parts.back().sourceWidth == 1808);
    // Culling must retain the global phase, not restart at the viewport edge.
    parts.clear();
    ForEachRepeatXTile(-quad + 1800.f, quad, tile, 2048, 1280.f, true,
                       [&](RepeatXTile part) { parts.push_back(part); });
    assert(parts.size() == 2);
    assert(parts.front().offset > 0 && parts.back().offset > 0);
    std::cout << "PASS: flipped whole-quad repeat phase and viewport culling\n";
}
