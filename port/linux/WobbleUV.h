#pragma once

namespace deathleak::render {
struct WobbleUV { float u, v; };

// D3D scrolls the vertex UV before PixelWobble.fx perturbs V, then U.
// GL uses the same authored coordinates until its final 1-V texture upload conversion.
inline WobbleUV PixelWobbleVertexUV(bool flipX, bool flipY,
                                    float shiftX, float shiftY,
                                    bool right, bool bottom) {
    const float u = (right == flipX ? 0.f : 1.f) + shiftX;
    const float v = (bottom == flipY ? 1.f : 0.f) + shiftY;
    return {u, v};
}
}
