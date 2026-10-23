#pragma once
#include "FoliageSway.h"
#include "WobbleUV.h"
#include <cmath>

namespace deathleak::render {
struct EffectVertex { float x, y, u, v; };

// Exact screen-space construction used by the GL pixel-wobble quad; its UVs
// remain D3D-oriented until the fragment shader's final V conversion.
inline EffectVertex PixelWobbleQuadVertex(float cx, float cy, float width, float height,
                                         float degrees, bool flipX, bool flipY,
                                         float shiftX, float shiftY,
                                         bool right, bool bottom) {
    constexpr float rad = 3.14159265358979323846f / 180.f;
    const float angle = degrees * rad, cs = std::cos(angle), sn = std::sin(angle);
    const float x = (right ? .5f : -.5f) * width;
    const float y = (bottom ? .5f : -.5f) * height;
    const auto uv = PixelWobbleVertexUV(flipX, flipY, shiftX, shiftY, right, bottom);
    return {cx + x * cs - y * sn, cy + x * sn + y * cs, uv.u, uv.v};
}

// Source labels are BL/BR/TR/TL, with source Y up and V=0 at bottom.
// The Windows World transform precedes X-then-Y shader sway.
inline SwayPoint FoliageQuadVertex(float cx, float cy, float width, float height,
                                  float degrees, bool flipX, bool flipY,
                                  float sourceX, float sourceY, SwayPoint intensity,
                                  SwayPoint worldCenter, float time,
                                  float timeMultiplier, float globalTimeMultiplier,
                                  float scale = 2.f / 3.f) {
    constexpr float rad = 3.14159265358979323846f / 180.f;
    const float angle = degrees * rad, cs = std::cos(angle), sn = std::sin(angle);
    const float x = sourceX * width * .5f * (flipX ? -1.f : 1.f);
    const float y = -sourceY * height * .5f * (flipY ? -1.f : 1.f);
    const float rx = x * cs - y * sn, ry = x * sn + y * cs;
    const SwayPoint world{worldCenter.x + rx / scale, worldCenter.y - ry / scale};
    const auto moved = FoliageSwayWorldVertex(world, intensity, time,
                                             timeMultiplier, globalTimeMultiplier);
    return {cx + rx + (moved.x - world.x) * scale,
            cy + ry - (moved.y - world.y) * scale};
}
}
