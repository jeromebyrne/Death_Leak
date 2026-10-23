// Pure CPU adapter; no SDL/GL initialization or renderer build.
#include "../linux/ParallaxUVCommand.h"
#include "../linux/UVWrap.h"
#ifdef NDEBUG
#error "assertions required"
#endif
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
#include <vector>
using namespace deathleak::render;
namespace {
void near(float a, double e, double tol=2e-6) {
    assert(std::fabs(static_cast<double>(a)-e)<=tol);
}
int oldCut(float s) { return static_cast<int>((s-std::floor(s))*2048); }
bool sameFloat(float actual, float expected) {
    return std::isnan(expected) ? std::isnan(actual) : actual == expected;
}
void preservedLegacy(const ParallaxUVScrollInput& in,
                     const ParallaxUVBinding& binding) {
    const auto c=PrepareParallaxUVCommand(in,binding);
    assert(c.kind==ParallaxUVCommandKind::LegacyInteger);
    assert(sameFloat(c.destination.x,in.destination.x));
    assert(sameFloat(c.destination.y,in.destination.y));
    assert(sameFloat(c.destination.width,in.destination.width));
    assert(sameFloat(c.destination.height,in.destination.height));
    assert(sameFloat(c.shiftX,in.shiftX) && sameFloat(c.shiftY,in.shiftY));
    assert(c.flipX==in.flipX && c.flipY==in.flipY);
}
struct Sink {
    std::vector<ParallaxUVCommand> quads;
    std::vector<UVWrapSegment> segments;
    int copies=0;
    void submit(const ParallaxUVCommand& c,int width,int height) {
        if(c.kind==ParallaxUVCommandKind::FloatQuad) quads.push_back(c);
        else if(c.kind==ParallaxUVCommandKind::LegacyInteger)
            ForEachUVWrapSegment(height,c.shiftY,c.flipY,[&](UVWrapSegment y) {
                ForEachUVWrapSegment(width,c.shiftX,c.flipX,[&](UVWrapSegment x) {
                    segments.push_back(y); segments.push_back(x); ++copies;
                });
            });
    }
};
void forest(bool flipped,int height) {
    const double camera[]={1000.,1000.5,1000.6,1000.7};
    const double phase[]={.690000,.690345,.690414,.690483};
    const int cuts[]={1413,1413,1413,1414}, flippedCuts[]={634,634,634,633};
    const ParallaxUVRect dest{17.25f,39.5f,2048.f*(2.f/3.f),height*(2.f/3.f)};
    const ParallaxUVBinding binding{true,true,1.f,1.f,height==677,true};
    ParallaxUVCommand c[4];
    for(int i=0;i<4;++i) {
        const float shift=static_cast<float>(camera[i]*.000690*(flipped?-1.:1.));
        c[i]=PrepareParallaxUVCommand({dest,shift,0.f,flipped,false,true,true},binding);
        Sink sink; sink.submit(c[i],2048,height);
        assert(sink.quads.size()==1 && sink.copies==0 && sink.segments.empty());
        const auto& q=sink.quads.front();
        assert(q.destination.x==dest.x && q.destination.y==dest.y);
        assert(q.destination.width==dest.width && q.destination.height==dest.height);
        near(q.uv[0].u,flipped?1-phase[i]:phase[i]);
        near(q.uv[1].u,flipped?-phase[i]:1+phase[i]);
        near(q.uv[1].u-q.uv[0].u,flipped?-1:1);
        near(q.uv[2].u-q.uv[3].u,flipped?-1:1);
        near(q.uv[0].v,0); near(q.uv[2].v,1);
        assert(oldCut(shift)==(flipped?flippedCuts[i]:cuts[i]));
    }
    const double sign=flipped?-1.:1.;
    for(int corner=0;corner<4;++corner) {
        near(c[1].uv[corner].u-c[0].uv[corner].u,sign*.000345);
        near(c[3].uv[corner].u-c[2].uv[corner].u,sign*.000069);
    }
    near((c[1].uv[0].u-c[0].uv[0].u)*2048,sign*.70656,.0002);
    near((c[3].uv[0].u-c[2].uv[0].u)*2048,sign*.141312,.0002);
}
void fallback() {
    const ParallaxUVScrollInput in{{17.25f,39.5f,1365.3333f,341.3333f},
                                    .690345f,.125f,false,false,true,true};
    const ParallaxUVBinding eligible{true,true,1.f,1.f,false,false};
    assert(PrepareParallaxUVCommand(in,eligible).kind==ParallaxUVCommandKind::FloatQuad);
    for(auto b : {ParallaxUVBinding{true,true,1.f,.75f,false,false},
                  ParallaxUVBinding{true,false,1.f,1.f,false,false},
                  ParallaxUVBinding{false,true,1.f,1.f,false,false},
                  ParallaxUVBinding{true,true,1.f,1.f,true,false}}) {
        const auto c=PrepareParallaxUVCommand(in,b);
        assert(c.kind==ParallaxUVCommandKind::LegacyInteger);
        assert(c.destination.x==in.destination.x && c.destination.width==in.destination.width);
        assert(c.shiftX==in.shiftX && c.shiftY==in.shiftY);
        assert(c.flipX==in.flipX && c.flipY==in.flipY);
        Sink actual;
        actual.submit(c,2048,512);
        assert(actual.quads.empty() && actual.copies==4 && actual.segments.size()==8);
        // Existing nested integer segmentation: Y 64..511 then 0..63,
        // and for each Y copy, X 1413..2047 then 0..1412.
        const int expectedSource[]={64,1413,64,0,0,1413,0,0};
        const int expectedLength[]={448,635,448,1413,64,635,64,1413};
        for(size_t i=0;i<actual.segments.size();++i) {
            assert(actual.segments[i].source==expectedSource[i]);
            assert(actual.segments[i].length==expectedLength[i]);
        }
    }
    auto excluded=in; excluded.fullTexture=false;
    assert(PrepareParallaxUVCommand(excluded,eligible).kind==ParallaxUVCommandKind::NotApplicable);
    excluded=in; excluded.unrotated=false;
    assert(PrepareParallaxUVCommand(excluded,eligible).kind==ParallaxUVCommandKind::NotApplicable);
    excluded=in; excluded.shiftX=excluded.shiftY=0.f;
    assert(PrepareParallaxUVCommand(excluded,eligible).kind==ParallaxUVCommandKind::NotApplicable);
}
void edgeGate() {
    const float inf=std::numeric_limits<float>::infinity();
    const float nan=std::numeric_limits<float>::quiet_NaN();
    const ParallaxUVScrollInput in{{-17.25f,39.5f,1365.3333f,341.3333f},
                                    .690345f,.125f,true,true,true,true};
    const ParallaxUVBinding eligible{true,true,1.f,1.f,false,false};
    // A finite shift stays on the unchanged integer-copy route for every
    // rejected binding, including bad signs, missing spans and backend failure.
    for(const auto b : {ParallaxUVBinding{false,true,1.f,1.f,false,false},
                        ParallaxUVBinding{true,false,1.f,1.f,false,false},
                        ParallaxUVBinding{true,true,0.f,1.f,false,false},
                        ParallaxUVBinding{true,true,1.f,0.f,false,false},
                        ParallaxUVBinding{true,true,-1.f,1.f,false,false},
                        ParallaxUVBinding{true,true,1.f,-1.f,false,false},
                        ParallaxUVBinding{true,true,.75f,1.f,false,false},
                        ParallaxUVBinding{true,true,1.f,1.00001f,false,false},
                        ParallaxUVBinding{true,true,inf,1.f,false,false},
                        ParallaxUVBinding{true,true,1.f,nan,false,false}}) {
        preservedLegacy(in,b);
        Sink sink; sink.submit(PrepareParallaxUVCommand(in,b),2048,512);
        assert(sink.quads.empty() && sink.copies==4 && sink.segments.size()==8);
    }
    // UVWrap's integer conversion is undefined for nonfinite shifts; this
    // preparation test verifies only the fallback command's preserved inputs.
    for(const float bad : {inf,-inf,nan}) {
        auto shifted=in; shifted.shiftX=bad;
        preservedLegacy(shifted,eligible);
        shifted=in; shifted.shiftY=bad;
        preservedLegacy(shifted,eligible);
    }
}
}
int main() {
    forest(false,512); forest(false,677); forest(true,512); forest(true,677);
    fallback();
    edgeGate();
    std::cout<<"PASS: fractional parallax command CPU seam\n";
}
