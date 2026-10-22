#pragma once

#include "Renderer.h"
#include <cmath>

namespace deathleak::render {

// Screen-space bounds of the same centered quad rotated by the SDL renderer.
// Conservative AABB admission; pixels beyond the viewport are clipped by GL.
inline bool SpriteMayIntersectViewport(const Sprite& sprite, float viewportWidth = 1280.f,
                                       float viewportHeight = 720.f) {
    const auto& r = sprite.destination;
    const float centerX = r.x + r.width * .5f;
    const float centerY = r.y + r.height * .5f;
    const float radians = sprite.rotationDegrees * (3.14159265358979323846f / 180.f);
    const float cosine = std::abs(std::cos(radians));
    const float sine = std::abs(std::sin(radians));
    const float halfX = (r.width * cosine + r.height * sine) * .5f;
    const float halfY = (r.width * sine + r.height * cosine) * .5f;
    return centerX + halfX >= 0.f && centerX - halfX <= viewportWidth &&
           centerY + halfY >= 0.f && centerY - halfY <= viewportHeight;
}

} // namespace deathleak::render
