#pragma once

#include <algorithm>
#include <cmath>

namespace deathleak::render {

// One SDL copy of a repeated source texture. Offsets are relative to the
// authored destination quad, not to the viewport.
struct RepeatXTile {
    float offset;
    float width;
    int sourceWidth;
};

// The source flips the entire quad after assigning UVs 0..(quad/texture).
// A flipped partial repeat therefore belongs at the quad's left edge.
template <typename DrawTile>
void ForEachRepeatXTile(float destinationX, float destinationWidth,
                        float tileWidth, int textureWidth, float viewportWidth,
                        bool flipX, DrawTile drawTile) {
    if (destinationWidth <= 0 || tileWidth <= 0 || textureWidth <= 0) return;
    const int count = static_cast<int>(std::ceil(destinationWidth / tileWidth));
    const float anchor = flipX ? destinationX + destinationWidth : destinationX;
    const int first = std::clamp(static_cast<int>(std::floor(
        (flipX ? anchor - viewportWidth : -anchor) / tileWidth)), 0, count);
    const int last = std::clamp(static_cast<int>(std::ceil(
        (flipX ? anchor : viewportWidth - anchor) / tileWidth)), 0, count);
    for (int i = first; i < last; ++i) {
        const float offset = flipX
            ? std::max(0.f, destinationWidth - (i + 1) * tileWidth)
            : i * tileWidth;
        const float right = flipX ? destinationWidth - i * tileWidth
                                  : std::min(destinationWidth, offset + tileWidth);
        const float visibleWidth = right - offset;
        if (visibleWidth <= 0) continue;
        const int sourceWidth = std::clamp(static_cast<int>(
            std::lround(textureWidth * visibleWidth / tileWidth)), 1, textureWidth);
        drawTile(RepeatXTile{offset, visibleWidth, sourceWidth});
    }
}

} // namespace deathleak::render
