#pragma once

#include <array>
#include <cmath>

namespace deathleak::render {

struct ParallaxUVCorner { float u, v; };
using ParallaxUVCorners = std::array<ParallaxUVCorner, 4>; // TL, TR, BR, BL

// The caller supplies the already SDL-facing signed shift. A flip changes the
// screen-edge span, not the sign of that shift. Only finite inputs are valid.
inline ParallaxUVCorners MakeParallaxUVCorners(float shiftX, float shiftY,
                                              bool flipX, bool flipY) {
    const float u = shiftX - std::floor(shiftX);
    const float v = shiftY - std::floor(shiftY);
    const float right = u + (flipX ? -1.f : 1.f);
    const float bottom = v + (flipY ? -1.f : 1.f);
    return {{{u, v}, {right, v}, {right, bottom}, {u, bottom}}};
}

// This decision is deliberately independent of SDL/GL: the caller measures
// actual bound spans and capability, then falls back to integer SDL copies if
// direct repetition could sample padding or unsupported NPOT allocation.
struct ParallaxUVBinding {
    bool glRenderer = false;
    bool bindSucceeded = false;
    float spanX = 0.f, spanY = 0.f;
    bool npotTexture = false;
    bool npotRepeatSupported = false;
};
inline bool ParallaxUVDirectEligible(const ParallaxUVBinding& b) {
    auto unit = [](float s) {
        return std::isfinite(s) && s > 0.f && std::fabs(s - 1.f) <= 1e-6f;
    };
    return b.glRenderer && b.bindSucceeded && unit(b.spanX) && unit(b.spanY)
        && (!b.npotTexture || b.npotRepeatSupported);
}

enum class ParallaxUVPath { DamageBurst, PixelWobble, FoliageSway,
                            RepeatX, DirectFloat, LegacyInteger, Ordinary };
struct ParallaxUVDispatch {
    bool damageBurst = false;    // effect active and program available
    bool pixelWobble = false;    // effect active and program available
    bool foliageSway = false;
    bool repeatX = false;
    bool hasSourceRect = false;
    bool rotated = false;
    bool uvScroll = false;
};
// Mirrors DrawSprite's effect priority. Eligibility is consulted only after
// the full-texture, unrotated UV-scroll branch wins dispatch.
inline ParallaxUVPath SelectParallaxUVPath(const ParallaxUVDispatch& d,
                                          const ParallaxUVBinding& b) {
    if (d.damageBurst) return ParallaxUVPath::DamageBurst;
    if (d.pixelWobble && !d.hasSourceRect) return ParallaxUVPath::PixelWobble;
    if (d.foliageSway && !d.hasSourceRect) return ParallaxUVPath::FoliageSway;
    if (d.repeatX && !d.rotated && !d.hasSourceRect) return ParallaxUVPath::RepeatX;
    if (d.uvScroll && !d.rotated && !d.hasSourceRect)
        return ParallaxUVDirectEligible(b) ? ParallaxUVPath::DirectFloat
                                           : ParallaxUVPath::LegacyInteger;
    return ParallaxUVPath::Ordinary;
}

} // namespace deathleak::render
