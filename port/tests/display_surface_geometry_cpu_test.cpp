#include "../linux/DisplaySurfaceGeometry.h"
#include "../linux/LogicalViewport.h"
#include <cassert>
#include <cmath>
using deathleak::linux_port::DisplaySurfaceGeometry;
using deathleak::linux_port::LogicalViewport;
int main() {
    DisplaySurfaceGeometry g{1280,720,60,1280,720,1280,720,1280,720,60,true,false};
    assert(g.Matches());
    g.outputHeight = 688; assert(!g.Matches()); // observed prepatch bad capture
    g.outputHeight = 720; g.windowHeight = 688; assert(!g.Matches());
    g.windowHeight = 720; g.modeRefreshHz = 120; assert(!g.Matches());
    g.modeRefreshHz = 60; g.modeHeight = 900; assert(!g.Matches());
    g.modeHeight = 720; g.fullscreen = false; assert(!g.Matches());
    g.fullscreen = true; g.minimized = true; assert(!g.Matches());
    g = {1440,810,120,1440,810,1440,810,1440,810,120,true,false};
    assert(g.Matches());
    auto v = LogicalViewport::ForSize(1440,810);
    assert(v.x == 0 && v.y == 0 && v.width == 1440 && v.height == 810);
    auto p = v.WindowToLogical(720,405,1440,810,1440,810);
    assert(std::fabs(p.x-640) < 0.001f && std::fabs(p.y-360) < 0.001f);
    v = LogicalViewport::ForSize(1280,688);
    p = v.WindowToLogical(640,344,1280,688,1280,688);
    assert(std::fabs(p.x-640) < 1.f && std::fabs(p.y-360) < 1.f);
    // Capture validity is independent of valid mapping through a changed viewport.
    g = {1280,720,60,1280,688,1280,688,1280,720,60,true,false};
    assert(!g.Matches());
}
