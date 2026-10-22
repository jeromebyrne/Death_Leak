#pragma once
#include <cmath>

namespace deathleak::render {
struct SwayPoint { float x, y; };

// FoliageSway.fx VS: displace the world vertex in X, then use that updated X
// for the Y sine. Intensities are the source vertex NORMAL.xy values.
inline SwayPoint FoliageSwayWorldVertex(SwayPoint world, SwayPoint intensity,
                                       float time, float timeMultiplier,
                                       float globalTimeMultiplier = 1.f) {
    const float phase = time * timeMultiplier * globalTimeMultiplier;
    world.x += std::sin(world.y * .1f + phase) * intensity.x;
    world.y += std::sin(world.x * .1f + phase) * intensity.y;
    return world;
}
}
