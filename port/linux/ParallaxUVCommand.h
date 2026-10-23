#pragma once
#include "ParallaxUV.h"
#include <cmath>

namespace deathleak::render {

// Pure preparation for DrawSprite's existing full-texture UV-scroll branch.
// The caller measures binding spans; this code does no SDL/GL work.
struct ParallaxUVRect { float x, y, width, height; };
struct ParallaxUVScrollInput {
    ParallaxUVRect destination;
    float shiftX, shiftY; // Already signed in SDL screen-facing convention.
    bool flipX, flipY;
    bool fullTexture, unrotated;
};
enum class ParallaxUVCommandKind { NotApplicable, FloatQuad, LegacyInteger };
struct ParallaxUVCommand {
    ParallaxUVCommandKind kind;
    ParallaxUVRect destination;
    ParallaxUVCorners uv{}; // TL, TR, BR, BL; only for FloatQuad.
    float shiftX, shiftY; // Original inputs for unchanged integer fallback.
    bool flipX, flipY;
};
inline ParallaxUVCommand PrepareParallaxUVCommand(
    const ParallaxUVScrollInput& input, const ParallaxUVBinding& binding) {
    ParallaxUVCommand command{ParallaxUVCommandKind::NotApplicable,
        input.destination, {}, input.shiftX, input.shiftY, input.flipX, input.flipY};
    if (!input.fullTexture || !input.unrotated ||
        (input.shiftX == 0.f && input.shiftY == 0.f))
        return command;
    command.kind = ParallaxUVCommandKind::LegacyInteger;
    if (std::isfinite(input.shiftX) && std::isfinite(input.shiftY) &&
        ParallaxUVDirectEligible(binding)) {
        command.kind = ParallaxUVCommandKind::FloatQuad;
        command.uv = MakeParallaxUVCorners(input.shiftX, input.shiftY,
                                           input.flipX, input.flipY);
    }
    return command;
}
} // namespace deathleak::render
