#pragma once

namespace deathleak::render {
// Windows applies authored X/Y scale after rotation. SDL/custom quads reflect
// the UV-labeled source before clockwise screen rotation. Each authored axis
// reflection reverses the angle sign. flipY is authored, not SDL inverse-V.
inline float OrdinarySpriteRotationDegrees(float radians, bool flipX, bool flipY) {
    constexpr float kRadiansToDegrees = 180.f / 3.14159265358979323846f;
    return radians * kRadiansToDegrees * (flipX == flipY ? -1.f : 1.f);
}
}
