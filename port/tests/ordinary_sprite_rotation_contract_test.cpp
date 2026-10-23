#include "../render/OrdinarySpriteRotation.h"
#include <array>
#include <cassert>
#include <cmath>
#include <iostream>

struct Point { double x, y; };
struct Case { const char* level; int id; double x, y, width, height, radians; bool flipX, flipY; };
static double distance(Point a, Point b) { return std::hypot(a.x-b.x, a.y-b.y); }
// Windows source labels are U=0/1 left/right, V=0/1 bottom/top.
static Point windowsCorner(const Case& c, int u, int v) {
    const double x = (u ? .5 : -.5) * c.width;
    const double y = (v ? -.5 : .5) * c.height;
    const double cs = std::cos(c.radians), sn = std::sin(c.radians);
    return {c.x + (c.flipX ? -1 : 1) * (cs*x + sn*y),
            c.y + (c.flipY ? -1 : 1) * (-sn*x + cs*y)};
}
static Point sdlCorner(const Case& c, int u, int v, double degrees) {
    // SDL copy's Y flag is !authoredFlipY: V0 bottom unless authored Y flips.
    const double x = (u ? .5 : -.5) * c.width * (c.flipX ? -1 : 1);
    const double y = (v ? -.5 : .5) * c.height * (c.flipY ? -1 : 1);
    const double a = degrees * 3.14159265358979323846 / 180.;
    return {c.x + std::cos(a)*x - std::sin(a)*y,
            c.y + std::sin(a)*x + std::cos(a)*y};
}
int main() {
    const std::array<Case, 8> cases{{
        {"waterfall_top", 32, -92.014648, 2790, 674.042114, 184.308380, .180110, true, false},
        {"waterfall_top", 33, 803.985352, 2813, 674.042114, 184.308380, -.279275, true, false},
        {"forest_exploration_1", 14, -2212.014648, 3210, 512, 512, -1.519999, true, false},
        {"forest_exploration_1", 15, 2796.985352, 3207, 512, 512, -1.469999, false, false},
        {"underwater_cave_4", 15, -2404, 651.627930, 449.339661, 449.339661, -1.680533, true, true},
        {"underwater_cave_4", 16, 1243, 665.627930, 449.339661, 449.339661, -1.680533, false, true},
        {"synthetic", 0, 45, -12, 311, 167, .37, false, true},
        {"synthetic", 1, 45, -12, 311, 167, .37, true, true},
    }};
    for (const auto& c : cases) {
        const double degrees = deathleak::render::OrdinarySpriteRotationDegrees(
            static_cast<float>(c.radians), c.flipX, c.flipY);
        const double expectedSign = c.flipX == c.flipY ? -1 : 1;
        assert(std::abs(degrees - expectedSign*c.radians*180/3.14159265358979323846) < 2e-5);
        bool wrongSignFails = false;
        for (int v=0; v<2; ++v) for (int u=0; u<2; ++u) {
            const Point expected = windowsCorner(c,u,v);
            assert(distance(expected,sdlCorner(c,u,v,degrees)) < .001);
            wrongSignFails |= distance(expected,sdlCorner(c,u,v,-degrees)) > 1;
        }
        assert(wrongSignFails);
        std::cout << "PASS ordinary " << c.level << " ID" << c.id << " four UV labels\n";
    }
}
