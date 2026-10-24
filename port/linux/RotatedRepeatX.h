#pragma once
#include <cmath>

namespace deathleak::render {
struct RotatedRepeatXVertex { float x, y, u, v; };
struct RotatedRepeatXQuad { RotatedRepeatXVertex vertices[4]; float span; };

// Full ordinary-sprite quad. The incoming base width is already world-to-screen scaled; undo that
// 2/3 scale to recover the authored/source U ratio, independent of zoom.
inline bool MakeRotatedRepeatXQuad(float x, float y, float width, float height,
                                   float baseProjectedWidth, int textureWidth,
                                   float degrees, bool flipX, bool flipY,
                                   RotatedRepeatXQuad& out) {
    if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(width) ||
        !std::isfinite(height) || !std::isfinite(baseProjectedWidth) ||
        !std::isfinite(degrees) || width <= 0 || height <= 0 ||
        baseProjectedWidth <= 0 || textureWidth <= 0) return false;
    const float span = baseProjectedWidth / (textureWidth * (2.f / 3.f));
    if (!std::isfinite(span) || span <= 0) return false;
    const float radians = degrees * 3.14159265358979323846f / 180.f;
    const float cosine = std::cos(radians), sine = std::sin(radians);
    const float cx = x + width * .5f, cy = y + height * .5f;
    const float leftU = flipX ? span : 0.f, rightU = flipX ? 0.f : span;
    const float topV = flipY ? 0.f : 1.f, bottomV = 1.f - topV;
    const float localX[4] = {-width*.5f, width*.5f, width*.5f, -width*.5f};
    const float localY[4] = {-height*.5f, -height*.5f, height*.5f, height*.5f};
    const float us[4] = {leftU, rightU, rightU, leftU};
    const float vs[4] = {topV, topV, bottomV, bottomV};
    out.span = span;
    for (int i = 0; i < 4; ++i) {
        out.vertices[i] = {cx + localX[i]*cosine-localY[i]*sine,
                           cy + localX[i]*sine+localY[i]*cosine, us[i], vs[i]};
    }
    return true;
}
} // namespace deathleak::render
