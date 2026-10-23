// Pure CPU geometry/dispatch contract; no SDL init, GL context, Xorg or GPU.
#include "../linux/ParallaxUV.h"
#include "../linux/UVWrap.h"
#ifdef NDEBUG
#error "fractional_parallax_uv_test requires enabled assertions"
#endif
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>

using namespace deathleak::render;
namespace {
void near(double actual, double expected, double tolerance = 2e-6) {
    assert(std::fabs(actual - expected) <= tolerance);
}
void geometry(const ParallaxUVCorners& c, bool fx, bool fy) {
    near(c[1].u-c[0].u, fx ? -1 : 1);
    near(c[2].u-c[3].u, fx ? -1 : 1);
    near(c[3].v-c[0].v, fy ? -1 : 1);
    near(c[2].v-c[1].v, fy ? -1 : 1);
    near(c[0].v,c[1].v); near(c[3].v,c[2].v);
    near(c[0].u,c[3].u); near(c[1].u,c[2].u);
}
int oldCut(float shift, int pixels = 2048) {
    return static_cast<int>((shift-std::floor(shift))*pixels);
}
double circularDelta(double later, double earlier) {
    double d = later-earlier;
    if (d < -.5) d += 1;
    if (d > .5) d -= 1;
    return d;
}
// Reconstruct the old nearest-neighbor source order at exact integer cuts.
int legacyNearest(int pixels, float shift, bool flipped, int screenPixel) {
    int source = -1;
    ForEachUVWrapSegment(pixels, shift, flipped, [&](UVWrapSegment segment) {
        const int start = static_cast<int>(segment.offset*pixels);
        if (screenPixel >= start && screenPixel < start+segment.length) {
            const int local = screenPixel-start;
            source = segment.source + (flipped ? segment.length-1-local : local);
        }
    });
    return source;
}
void forest(bool flipX, double textureHeight) {
    assert(textureHeight == 512 || textureHeight == 677);
    constexpr double width = 2048.0, destWidth = width*2.0/3.0;
    const double cameras[] = {1000.0,1000.5,1000.6,1000.7};
    const double unflipped[] = {.690000,.690345,.690414,.690483};
    const int cuts[] = {1413,1413,1413,1414};
    const int flippedCuts[] = {634,634,634,633};
    ParallaxUVCorners c[4];
    for (int i=0;i<4;++i) {
        const float shift = static_cast<float>(cameras[i]*.000690*(flipX ? -1.0 : 1.0));
        c[i]=MakeParallaxUVCorners(shift,0.f,flipX,false);
        geometry(c[i],flipX,false);
        near(c[i][0].u, flipX ? 1-unflipped[i] : unflipped[i]);
        near(c[i][1].u, flipX ? -unflipped[i] : 1+unflipped[i]);
        near(c[i][0].v,0); near(c[i][2].v,1);
        near(c[i][0].u*width, (flipX ? 1-unflipped[i] : unflipped[i])*width, .0002);
        assert(oldCut(shift)==(flipX ? flippedCuts[i] : cuts[i]));
    }
    const double sign = flipX ? -1.0 : 1.0;
    for (int corner=0;corner<4;++corner) {
        near(c[1][corner].u-c[0][corner].u,sign*.000345);
        near(c[3][corner].u-c[2][corner].u,sign*.000069);
    }
    near((c[1][0].u-c[0][0].u)*width,sign*.70656,.0002);
    near((c[3][0].u-c[2][0].u)*width,sign*.141312,.0002);
    near(std::fabs((c[1][0].u-c[0][0].u)*destWidth),.47104,.0002);
    near(std::fabs((c[3][0].u-c[2][0].u)*destWidth),.094208,.0002);
    // With the authored positive shift, increasing U moves a fixed feature left;
    // the flipped signed shift plus reversed edge also moves it left.
    assert((flipX ? -1 : 1)*(c[1][0].u-c[0][0].u)>0);
    // Authored 2/3-scale destination contract at a held camera; this is a
    // synthetic geometry check, not an observation of DrawSprite's output.
    const double destination[] = {0, 0, destWidth, textureHeight*2.0/3.0};
    const double expectedDestination[] = {0, 0, 1365.3333333333333,
        textureHeight == 512 ? 341.3333333333333 : 451.3333333333333};
    for (int i=0;i<4;++i) near(destination[i],expectedDestination[i]);
}
}
int main() {
    // Both forest IDs share X geometry; ID40's distinct height is a binding gate.
    forest(false,512); // ID37
    forest(false,677); // ID40
    forest(true,512);
    const float shifts[] = {0.f,.999f,1.f,1.001f,-.25f,-1.25f};
    const double phases[] = {0,.999,0,.001,.75,.75};
    for (int i=0;i<6;++i) {
        const auto c=MakeParallaxUVCorners(shifts[i],0,false,false);
        geometry(c,false,false);
        near(c[0].u,phases[i]); near(c[1].u,phases[i]+1);
    }
    near(circularDelta(phases[2],phases[1]),.001);
    const auto fx=MakeParallaxUVCorners(-.25f,0,true,false);
    near(fx[0].u,.75); near(fx[1].u,-.25);
    const auto fxPositive=MakeParallaxUVCorners(.25f,0,true,false);
    near(fxPositive[0].u,.25); near(fxPositive[1].u,-.75);
    const auto fy=MakeParallaxUVCorners(0,.25f,false,false);
    near(fy[0].v,.25); near(fy[2].v,1.25);
    const auto fyFlipped=MakeParallaxUVCorners(0,-.25f,false,true);
    near(fyFlipped[0].v,.75); near(fyFlipped[2].v,-.25);
    const auto both=MakeParallaxUVCorners(-.25f,-.25f,true,true);
    const double expected[][2]={{.75,.75},{-.25,.75},{-.25,-.25},{.75,-.25}};
    geometry(both,true,true);
    for (int i=0;i<4;++i) {near(both[i].u,expected[i][0]); near(both[i].v,expected[i][1]);}
    // Exact-aligned phase only: nearest sample order matches legacy segments.
    for (bool flipped : {false,true}) for (float phase : {.25f,-.25f}) {
        const auto axis=MakeParallaxUVCorners(phase,phase,flipped,flipped);
        for (int pixel=0;pixel<8;++pixel) {
            const float u=axis[0].u+(axis[1].u-axis[0].u)*(pixel+.5f)/8.f;
            const float v=axis[0].v+(axis[3].v-axis[0].v)*(pixel+.5f)/8.f;
            const int expectedSource=legacyNearest(8,phase,flipped,pixel);
            assert(expectedSource>=0);
            const int uSource=(static_cast<int>(std::floor(u*8))%8+8)%8;
            const int vSource=(static_cast<int>(std::floor(v*8))%8+8)%8;
            assert(uSource==expectedSource && vSource==expectedSource);
        }
    }
    ParallaxUVBinding binding{true,true,1.f,1.f,false,false};
    assert(ParallaxUVDirectEligible(binding)); // ID37, and ID40 if NPOT repeat works
    binding.npotTexture=true; binding.npotRepeatSupported=true;
    assert(ParallaxUVDirectEligible(binding));
    auto fallback=[&](float x,float y) {binding.spanX=x; binding.spanY=y; assert(!ParallaxUVDirectEligible(binding));};
    fallback(1.f,.75f); fallback(.75f,1.f); fallback(1.f,.9999f);
    fallback(0.f,1.f);
    fallback(std::numeric_limits<float>::quiet_NaN(),1.f);
    fallback(1.f,std::numeric_limits<float>::infinity());
    binding.spanX=.9999999f; binding.spanY=1.f;
    assert(ParallaxUVDirectEligible(binding));
    binding.spanX=1.f;
    binding.bindSucceeded=false; assert(!ParallaxUVDirectEligible(binding));
    binding.bindSucceeded=true; binding.glRenderer=false; assert(!ParallaxUVDirectEligible(binding));
    binding.glRenderer=true; binding.npotRepeatSupported=false; assert(!ParallaxUVDirectEligible(binding));
    binding.npotRepeatSupported=true;
    ParallaxUVDispatch d{}; d.uvScroll=true;
    assert(SelectParallaxUVPath(d,binding)==ParallaxUVPath::DirectFloat);
    binding.spanY=.75f;
    assert(SelectParallaxUVPath(d,binding)==ParallaxUVPath::LegacyInteger);
    binding.spanY=1.f;
    d.repeatX=true; assert(SelectParallaxUVPath(d,binding)==ParallaxUVPath::RepeatX);
    d.repeatX=false; d.foliageSway=true;
    assert(SelectParallaxUVPath(d,binding)==ParallaxUVPath::FoliageSway);
    d.pixelWobble=true; assert(SelectParallaxUVPath(d,binding)==ParallaxUVPath::PixelWobble);
    d.damageBurst=true; assert(SelectParallaxUVPath(d,binding)==ParallaxUVPath::DamageBurst);
    d.damageBurst=d.pixelWobble=d.foliageSway=false;
    d.hasSourceRect=true; assert(SelectParallaxUVPath(d,binding)==ParallaxUVPath::Ordinary);
    d.hasSourceRect=false; d.rotated=true;
    assert(SelectParallaxUVPath(d,binding)==ParallaxUVPath::Ordinary);
    d.rotated=false; d.uvScroll=false;
    assert(SelectParallaxUVPath(d,binding)==ParallaxUVPath::Ordinary);
    std::cout << "PASS: fractional parallax CPU corners, signed phase, seam, span gate and dispatch\n";
}
