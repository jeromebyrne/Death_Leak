#include "../linux/FoliageSway.h"
#include <cassert>
#include <cmath>
#include <iostream>
using deathleak::render::FoliageSwayWorldVertex;
int main() {
    const float time = .75f, local = 1.25f, normalWeather = 2.5f;
    const auto p = FoliageSwayWorldVertex({100.f, 30.f}, {7.f, 6.f},
                                           time, local, normalWeather);
    const float x = 100.f + std::sin(30.f * .1f + time * local * normalWeather) * 7.f;
    const float y = 30.f + std::sin(x * .1f + time * local * normalWeather) * 6.f;
    assert(std::abs(p.x - x) < .0001f && std::abs(p.y - y) < .0001f);
    const auto fixed = FoliageSwayWorldVertex({100.f, 30.f}, {0.f, 0.f}, time, local, normalWeather);
    assert(fixed.x == 100.f && fixed.y == 30.f);
    // D3D source vertex V=1 is world-top; SDL geometry V=0 is PNG-top.
    // The authored flip swaps which source vertex reaches each screen edge.
    assert(deathleak::render::FoliageScreenV(false, true) == 1.f);
    assert(deathleak::render::FoliageScreenV(false, false) == 0.f);
    assert(deathleak::render::FoliageScreenV(true, true) == 0.f);
    assert(deathleak::render::FoliageScreenV(true, false) == 1.f);
    std::cout << "PASS: FoliageSway.fx world-space vertex displacement\n";
}
