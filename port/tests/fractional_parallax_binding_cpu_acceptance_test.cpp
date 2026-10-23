// Standalone CPU acceptance of command UVs against the existing integer-copy
// segment mapping at exact texel phases; no SDL/GL renderer initialization.
#include "../linux/ParallaxUVCommand.h"
#include "../linux/UVWrap.h"
#ifdef NDEBUG
#error "assertions required"
#endif
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>
using namespace deathleak::render;
namespace {
struct AxisCopy { int source, length, start; };
std::vector<AxisCopy> copies(int pixels, float shift, bool flip) {
    std::vector<AxisCopy> result;
    ForEachUVWrapSegment(pixels, shift, flip, [&](UVWrapSegment s) {
        result.push_back({s.source, s.length, static_cast<int>(s.offset * pixels)});
    });
    return result;
}
int legacyTexel(const std::vector<AxisCopy>& segments, int screenPixel, bool flip) {
    for (const auto& s : segments)
        if (screenPixel >= s.start && screenPixel < s.start + s.length) {
            const int local = screenPixel - s.start;
            return s.source + (flip ? s.length - 1 - local : local);
        }
    assert(false && "uncovered destination pixel");
    return -1;
}
int uvTexel(float first, float last, int pixel, int extent) {
    const float t = (pixel + .5f) / extent;
    const float coordinate = first + (last - first) * t;
    const float phase = coordinate - std::floor(coordinate);
    return static_cast<int>(std::floor(phase * extent));
}
void exactMapping(float shiftX, float shiftY, bool flipX, bool flipY) {
    constexpr int width = 16, height = 8;
    const ParallaxUVRect destination{13.25f, -7.5f, 50.5f, 26.25f};
    const ParallaxUVBinding binding{true, true, 1.f, 1.f, false, false};
    const auto command = PrepareParallaxUVCommand(
        {destination, shiftX, shiftY, flipX, flipY, true, true}, binding);
    assert(command.kind == ParallaxUVCommandKind::FloatQuad);
    assert(command.destination.x == destination.x && command.destination.y == destination.y);
    assert(command.destination.width == destination.width && command.destination.height == destination.height);
    assert(command.uv[1].u - command.uv[0].u == (flipX ? -1.f : 1.f));
    assert(command.uv[3].v - command.uv[0].v == (flipY ? -1.f : 1.f));
    const auto xs = copies(width, shiftX, flipX);
    const auto ys = copies(height, shiftY, flipY);
    assert(xs.size() == 2 && ys.size() == 2);
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x) {
            // The existing renderer visits Y segments outside X segments.
            const int oldX = legacyTexel(xs, x, flipX);
            const int oldY = legacyTexel(ys, y, flipY);
            const int newX = uvTexel(command.uv[0].u, command.uv[1].u, x, width);
            const int newY = uvTexel(command.uv[0].v, command.uv[3].v, y, height);
            assert(newX == oldX && newY == oldY);
        }
}
void heldCutMovesOnBothAxes(bool flipX, bool flipY) {
    constexpr int width = 16, height = 8;
    const ParallaxUVBinding binding{true, true, 1.f, 1.f, false, false};
    const float sx = flipX ? -5.f / width : 5.f / width;
    const float sy = flipY ? -3.f / height : 3.f / height;
    const float dx = 1.f / (4 * width), dy = 1.f / (4 * height);
    auto make = [&](float x, float y) {
        return PrepareParallaxUVCommand({{1.f,2.f,3.f,4.f}, x, y,
                                         flipX, flipY, true, true}, binding);
    };
    const auto a = make(sx, sy), b = make(sx + dx, sy + dy);
    assert(a.kind == ParallaxUVCommandKind::FloatQuad && b.kind == a.kind);
    assert(a.uv[0].u != b.uv[0].u && a.uv[0].v != b.uv[0].v);
    assert(std::fabs((b.uv[0].u - a.uv[0].u) - dx) < 1e-6f);
    assert(std::fabs((b.uv[0].v - a.uv[0].v) - dy) < 1e-6f);
    const auto ax = copies(width, sx, flipX), bx = copies(width, sx + dx, flipX);
    const auto ay = copies(height, sy, flipY), by = copies(height, sy + dy, flipY);
    for (int x = 0; x < width; ++x) assert(legacyTexel(ax,x,flipX) == legacyTexel(bx,x,flipX));
    for (int y = 0; y < height; ++y) assert(legacyTexel(ay,y,flipY) == legacyTexel(by,y,flipY));
}
}
int main() {
    for (bool flipX : {false, true})
        for (bool flipY : {false, true}) {
            for (float sx : {5.f/16, -11.f/16})
                for (float sy : {3.f/8, -5.f/8})
                    exactMapping(sx, sy, flipX, flipY);
            heldCutMovesOnBothAxes(flipX, flipY);
        }
    std::cout << "PASS: command UV to segment CPU acceptance (both axes, all flips)\n";
}
