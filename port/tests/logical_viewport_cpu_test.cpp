#include "../linux/LogicalViewport.h"
#include <cassert>
#include <cmath>
using deathleak::linux_port::LogicalViewport;
static void Near(float a, float b) { assert(std::fabs(a-b) < 0.001f); }
int main() {
    const auto native = LogicalViewport::ForSize(1280, 720);
    assert(native.x == 0 && native.y == 0 && native.width == 1280 && native.height == 720);
    auto p = native.WindowToLogical(400, 511, 1280, 720, 1280, 720);
    Near(p.x, 400); Near(p.y, 511);
    const auto wide = LogicalViewport::ForSize(1440, 900);
    assert(wide.x == 0 && wide.y == 45 && wide.width == 1440 && wide.height == 810);
    p = wide.WindowToLogical(450, 620, 1440, 900, 1440, 900);
    Near(p.x, 400); Near(p.y, 511.11111f);
    p = wide.WindowToLogical(450, 20, 1440, 900, 1440, 900);
    assert(p.y < 0);
    p = wide.WindowToLogical(225, 310, 720, 450, 1440, 900);
    Near(p.x, 400); Near(p.y, 511.11111f);
    const auto eightK = LogicalViewport::ForSize(7680, 4320);
    assert(eightK.x == 0 && eightK.y == 0 && eightK.width == 7680 && eightK.height == 4320);
    p = eightK.WindowToLogical(3840, 2160, 7680, 4320, 7680, 4320);
    Near(p.x, 640); Near(p.y, 360);
    const auto empty = LogicalViewport::ForSize(0, 0);
    p = empty.WindowToLogical(1, 1, 0, 0, 0, 0);
    Near(p.x, -1); Near(p.y, -1);
}
