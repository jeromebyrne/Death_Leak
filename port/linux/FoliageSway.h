#pragma once
#include <cmath>

namespace deathleak::render {
struct SwayPoint { float x, y; };

// Sprite.cpp's source quad assigns V=0 to world-bottom and V=1 to
// world-top. SDL_RenderGeometry instead addresses the PNG's top row at V=0.
// Preserve the source sampling, including authored vertical flips.
inline float FoliageScreenV(bool authoredFlipY, bool screenTop) {
    return (screenTop != authoredFlipY) ? 1.f : 0.f;
}

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
