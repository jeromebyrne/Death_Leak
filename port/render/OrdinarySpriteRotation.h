#pragma once

namespace deathleak::render {
// Windows uses row-vector R*S*T: its X scale reflects after Z rotation.
// SDL reflects texture coordinates before its screen-space clockwise rotation.
// X reflection conjugates the rotation, so only unflipped ordinary sprites
// need the sign reversed to preserve UV-labeled corner positions.
inline float OrdinarySpriteRotationDegrees(float radians, bool flipX) {
    constexpr float kRadiansToDegrees = 180.f / 3.14159265358979323846f;
    return radians * kRadiansToDegrees * (flipX ? 1.f : -1.f);
}
}
