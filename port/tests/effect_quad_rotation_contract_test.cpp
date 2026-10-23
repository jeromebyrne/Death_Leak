#include "../linux/EffectQuadGeometry.h"
#include "../render/OrdinarySpriteRotation.h"
#include <array>
#include <cassert>
#include <cmath>
#include <iostream>
using namespace deathleak::render;
struct Point { double x, y; };
static double dist(Point a, Point b) { return std::hypot(a.x-b.x,a.y-b.y); }
// Original row-vector R*S, projected into screen Y-down, independent of SDL helper.
static Point windows(double cx, double cy, double width, double height,
                     double theta, bool fx, bool fy, int sx, int sy) {
    const double x=sx*width*.5, y=sy*height*.5;
    return {cx+(fx?-1:1)*(std::cos(theta)*x+std::sin(theta)*y),
            cy+(fy?-1:1)*(-std::sin(theta)*x+std::cos(theta)*y)};
}
int main() {
    constexpr double pi=3.14159265358979323846;
    // Forest-1 real fog controls plus synthetic X-only/both-flip coverage.
    struct Case { const char* label; double theta; bool fx,fy; };
    const std::array<Case,4> fog{{{"forest ID119",.11,false,false},
        {"forest ID122",.11,false,true},{"synthetic X",.31,true,false},
        {"synthetic XY",-.42,true,true}}};
    for (const auto& c:fog) {
        const float angle=OrdinarySpriteRotationDegrees(c.theta,c.fx,c.fy);
        if (!c.fx && !c.fy) assert(std::abs(angle+.11*180/pi)<1e-5);
        if (!c.fx && c.fy) assert(std::abs(angle-.11*180/pi)<1e-5);
        bool bad=false;
        for (int bottom=0;bottom<2;++bottom) for (int right=0;right<2;++right) {
            const auto vertex=PixelWobbleQuadVertex(123,234,1988,1290,angle,c.fx,c.fy,
                                                    .07f,.13f,right,bottom);
            const auto uv=PixelWobbleVertexUV(c.fx,c.fy,.07f,.13f,right,bottom);
            assert(vertex.u==uv.u && vertex.v==uv.v); // source UV identity/scroll
            const int sourceX=(right?1:-1)*(c.fx?-1:1);
            const int sourceY=(bottom?1:-1)*(c.fy?-1:1); // world source Y up
            const auto expected=windows(123,234,1988,1290,c.theta,c.fx,c.fy,sourceX,sourceY);
            assert(dist({vertex.x,vertex.y},expected)<.001);
            const auto wrong=PixelWobbleQuadVertex(123,234,1988,1290,-angle,c.fx,c.fy,
                                                   .07f,.13f,right,bottom);
            bad |= dist({wrong.x,wrong.y},expected)>1;
        }
        assert(bad);
        std::cout<<"PASS wobble "<<c.label<<" four UV-labeled corners\n";
    }
    // Waterfall-top leaf shadows 66/67, unflipped control, then remaining Y parities.
    const std::array<Case,5> leaves{{{"waterfall ID66",-.099903,true,false},
        {"waterfall ID67",.080501,true,false},{"unflipped",.27,false,false},
        {"Y only",-.33,false,true},{"XY",.22,true,true}}};
    const std::array<SwayPoint,4> amounts{{{1,2},{3,4},{8,9},{7,6}}}; // BL BR TR TL
    const std::array<std::pair<int,int>,4> source{{{-1,-1},{1,-1},{1,1},{-1,1}}};
    for (const auto& c:leaves) {
        const float angle=OrdinarySpriteRotationDegrees(c.theta,c.fx,c.fy);
        bool oldMinusFails=false;
        for (int i=0;i<4;++i) {
            const auto [sx,sy]=source[i];
            const auto expected=windows(400,300,2000,252,c.theta,c.fx,c.fy,sx,-sy);
            const auto p=FoliageQuadVertex(400,300,2000,252,angle,c.fx,c.fy,sx,sy,
                                          {0,0},{1500,2800},.75f,1,2.5f);
            assert(dist({p.x,p.y},expected)<.001);
            const auto wrong=FoliageQuadVertex(400,300,2000,252,-angle,c.fx,c.fy,sx,sy,
                                              {0,0},{1500,2800},.75f,1,2.5f);
            oldMinusFails |= dist({wrong.x,wrong.y},expected)>1;
            const auto moved=FoliageQuadVertex(400,300,2000,252,angle,c.fx,c.fy,sx,sy,
                                              amounts[i],{1500,2800},.75f,1,2.5f);
            const double wx=1500+(expected.x-400)*1.5;
            const double wy=2800-(expected.y-300)*1.5;
            const double phase=.75*2.5;
            const double dx=std::sin(wy*.1+phase)*amounts[i].x;
            const double dy=std::sin((wx+dx)*.1+phase)*amounts[i].y;
            assert(dist({moved.x,moved.y},{expected.x+dx/1.5,expected.y-dy/1.5})<.002);
            // A Y-sway evaluation using pre-X position must be detectably different.
            if (i==2) assert(std::abs(dy-std::sin(wx*.1+phase)*amounts[i].y)>1e-4);
        }
        assert(oldMinusFails);
        std::cout<<"PASS foliage "<<c.label<<" four source corners/intensities\n";
    }
}
