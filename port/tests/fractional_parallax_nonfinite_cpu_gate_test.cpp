// CPU-only command-to-legacy-sink safety gate. No SDL/GL initialization.
// Nonfinite command shifts must normalize safely at the legacy UV wrap sink.
#include "../linux/ParallaxUVCommand.h"
#include "../linux/UVWrap.h"
#ifdef NDEBUG
#error "assertions required"
#endif
#include <cassert>
#include <cmath>
#include <limits>
#include <vector>

using namespace deathleak::render;

namespace {
struct Copy { UVWrapSegment y, x; };
using Segments = std::vector<UVWrapSegment>;

Segments axis(int pixels, float shift, bool flip) {
    Segments result;
    ForEachUVWrapSegment(pixels, shift, flip,
                         [&](UVWrapSegment s) { result.push_back(s); });
    return result;
}
std::vector<Copy> submitLegacy(const ParallaxUVCommand& command,
                               int width, int height) {
    assert(command.kind == ParallaxUVCommandKind::LegacyInteger);
    std::vector<Copy> copies;
    ForEachUVWrapSegment(height, command.shiftY, command.flipY,
        [&](UVWrapSegment y) {
            ForEachUVWrapSegment(width, command.shiftX, command.flipX,
                [&](UVWrapSegment x) { copies.push_back({y, x}); });
        });
    return copies;
}
void equal(UVWrapSegment actual, UVWrapSegment expected) {
    assert(actual.source == expected.source);
    assert(actual.length == expected.length);
    assert(actual.offset == expected.offset);
}
void equal(const Segments& actual, const Segments& expected) {
    assert(actual.size() == expected.size());
    for (size_t i = 0; i < expected.size(); ++i) equal(actual[i], expected[i]);
}
void check(float x, float y, bool flipX, bool flipY, int width, int height) {
    const ParallaxUVScrollInput input{{3.f, 4.f, 15.f, 16.f},
                                      x, y, flipX, flipY, true, true};
    const ParallaxUVBinding binding{true, true, 1.f, 1.f, false, true};
    const auto command = PrepareParallaxUVCommand(input, binding);
    assert(command.kind == ParallaxUVCommandKind::LegacyInteger);
    assert(command.flipX == flipX && command.flipY == flipY);
    assert(command.destination.x == input.destination.x);
    assert(command.destination.y == input.destination.y);
    assert(command.destination.width == input.destination.width);
    assert(command.destination.height == input.destination.height);
    assert((std::isnan(x) && std::isnan(command.shiftX)) || command.shiftX == x);
    assert((std::isnan(y) && std::isnan(command.shiftY)) || command.shiftY == y);

    // A bad axis becomes exactly one zero-phase full-source segment. The
    // other axis must keep its ordinary finite segmentation and flip order.
    const Segments expectedX = std::isfinite(x)
        ? (flipX ? Segments{{0, 2, 0.f}, {2, 6, .25f}}
                 : Segments{{2, 6, 0.f}, {0, 2, .75f}})
        : Segments{{0, width, 0.f}};
    const Segments expectedY = std::isfinite(y)
        ? (flipY ? Segments{{0, 6, 0.f}, {6, 2, .75f}}
                 : Segments{{6, 2, 0.f}, {0, 6, .25f}})
        : Segments{{0, height, 0.f}};
    const auto xSegments = axis(width, command.shiftX, flipX);
    const auto ySegments = axis(height, command.shiftY, flipY);
    if (width > 0) equal(xSegments, expectedX);
    else assert(xSegments.empty());
    if (height > 0) equal(ySegments, expectedY);
    else assert(ySegments.empty());

    const auto copies = submitLegacy(command, width, height);
    assert(copies.size() == xSegments.size() * ySegments.size());
    size_t i = 0;
    for (auto expectedYSegment : ySegments)
        for (auto expectedXSegment : xSegments) {
            equal(copies[i].y, expectedYSegment);
            equal(copies[i].x, expectedXSegment);
            ++i;
        }
}
} // namespace

int main() {
    const float inf = std::numeric_limits<float>::infinity();
    const float nan = std::numeric_limits<float>::quiet_NaN();
    for (bool flipX : {false, true})
        for (bool flipY : {false, true})
            for (float bad : {nan, inf, -inf}) {
                check(bad, -.25f, flipX, flipY, 8, 8);
                check(.25f, bad, flipX, flipY, 8, 8);
                check(bad, bad, flipX, flipY, 8, 8);
                // Degenerate spans must visit no nested copies. The valid
                // opposite axis still preserves its own segments.
                for (int span : {0, -8}) {
                    check(bad, -.25f, flipX, flipY, span, 8);
                    check(.25f, bad, flipX, flipY, 8, span);
                    check(bad, bad, flipX, flipY, span, span);
                }
            }
}
