#pragma once
#include <algorithm>
#include <cmath>

namespace deathleak::render {
struct UVWrapSegment {
    int source = 0, length = 0;
    float offset = 0;
};

// Source interval and destination fraction for a one-span wrapped UV axis.
// SDL flips each copy locally: for a whole-quad flip the source intervals must
// also be visited in reverse order, or the seam jumps to the wrong edge.
template <class Visit>
void ForEachUVWrapSegment(int pixels, float shift, bool flipped, Visit visit) {
    if (pixels <= 0) return;
    const float phase = shift - std::floor(shift);
    const int cut = std::clamp(static_cast<int>(phase * pixels), 0, pixels - 1);
    if (cut == 0) { visit(UVWrapSegment{0, pixels, 0.f}); return; }
    if (flipped) {
        visit(UVWrapSegment{0, cut, 0.f});
        visit(UVWrapSegment{cut, pixels - cut, static_cast<float>(cut) / pixels});
    } else {
        visit(UVWrapSegment{cut, pixels - cut, 0.f});
        visit(UVWrapSegment{0, cut, static_cast<float>(pixels - cut) / pixels});
    }
}
}
