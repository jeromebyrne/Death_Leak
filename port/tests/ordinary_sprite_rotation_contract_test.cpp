#include "../render/OrdinarySpriteRotation.h"
#include <array>
#include <cassert>
#include <cmath>
#include <iostream>

struct Point { double x, y; };
struct Case { const char* level; int id; double x, y, width, height, radians; bool flipX; };

// UV-labeled source corners: U=0/1 from left/right, V=0/1 from top/bottom.
// D3D Sprite puts V=0 at world-bottom; screen Y therefore starts positive.
Point windowsCorner(const Case& c, int u, int v) {
    const double x = (u ? .5 : -.5) * c.width;
    const double y = (v ? -.5 : .5) * c.height;
    const double cs = std::cos(c.radians), sn = std::sin(c.radians);
    // D3DX row-vector R then authored X scale then translation, projected Y-down.
    return {c.x + (c.flipX ? -1 : 1) * (cs*x + sn*y),
            c.y - sn*x + cs*y};
}

Point sdlCorner(const Case& c, int u, int v, double degrees) {
    // SDL's inverse-V texture flag maps source V=0 to screen-bottom;
    // SDL_RenderCopyExF flips source X before rotating around the center.
    const double x = (u ? .5 : -.5) * c.width * (c.flipX ? -1 : 1);
    const double y = (v ? -.5 : .5) * c.height;
    const double angle = degrees * 3.14159265358979323846 / 180.;
    return {c.x + std::cos(angle)*x - std::sin(angle)*y,
            c.y + std::sin(angle)*x + std::cos(angle)*y};
}

int main() {
    const std::array<Case, 4> cases{{
        {"waterfall_top", 32, -92.014648, 2790, 674.042114, 184.308380, .180110, true},
        {"waterfall_top", 33, 803.985352, 2813, 674.042114, 184.308380, -.279275, true},
        {"forest_exploration_1", 14, -2212.014648, 3210, 512, 512, -1.519999, true},
        {"forest_exploration_1", 15, 2796.985352, 3207, 512, 512, -1.469999, false},
    }};
    constexpr double kDegrees = 180. / 3.14159265358979323846;
    for (const auto& c : cases) {
        bool legacyMismatch = false;
        for (int v = 0; v < 2; ++v) for (int u = 0; u < 2; ++u) {
            const auto expected = windowsCorner(c, u, v);
            const auto actual = sdlCorner(c, u, v,
                deathleak::render::OrdinarySpriteRotationDegrees(static_cast<float>(c.radians), c.flipX));
            const auto legacy = sdlCorner(c, u, v, c.radians * kDegrees);
            assert(std::hypot(expected.x-actual.x, expected.y-actual.y) < .001);
            legacyMismatch |= std::hypot(expected.x-legacy.x, expected.y-legacy.y) > 1.;
        }
        assert(legacyMismatch == !c.flipX);
        std::cout << "PASS " << c.level << " ID" << c.id
                  << " four UV corners; legacy mismatch=" << legacyMismatch << '\n';
    }
}
