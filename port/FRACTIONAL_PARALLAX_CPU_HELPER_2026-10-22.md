# Fractional parallax CPU helper — 2026-10-22

## Scope and result

Added only `port/linux/ParallaxUV.h` and `port/tests/fractional_parallax_uv_test.cpp`. No existing renderer, Makefile, shared checkpoint/status, or other source file was edited. No build, test execution, Docker, Xorg, or GPU operation was performed while the fog-mip worker owns that lane. Thus this is an implemented **unrun CPU gate**, not a passing test or a visual-parity claim.

## Helper contract

`MakeParallaxUVCorners` yields float TL/TR/BR/BL from the already SDL-facing signed shifts: `p=shift-floor(shift)`, with flipped trailing edge `p-1` and unflipped trailing edge `p+1`. Finite shifts are a caller precondition. There is no texel cut or rounding in this helper. `ParallaxUVDirectEligible` requires a successful SDL binding, GL renderer, finite positive X/Y spans each within `1e-6f` of one, and NPOT repeat support when relevant. The pure `SelectParallaxUVPath` models existing damage-burst → pixel-wobble → foliage → ordinary repeat-X → full-texture/unrotated UV-scroll precedence. This is CPU policy only; it cannot measure a live SDL binding or GL capability.

The standalone test covers ID37/ID40 common 2048-wide X phase at held camera 1000/1000.5 and cut crossing 1000.6/1000.7; unchanged synthetic destination, source texel and logical-pixel deltas, old-cut diagnostics; negative/signed phase, modulo-one seam continuity, X/Y and combined flips; exact-aligned nearest-source order versus `UVWrap.h`; exact/near-unit and padded/nonfinite/failed/non-GL/NPOT span decisions; and effect/ordinary-sprite branch precedence. It deliberately does not assert raster color, GPU filtering, physical seam sampling, or Windows pixel parity.

## Next integration step after ownership clears

Re-read then-current renderer and fog work. Wire this helper only into the full-texture, unrotated UV-scroll branch, preserving the integer `UVWrap.h` fallback for failed/non-unit binding or unsupported NPOT repeat. Add a focused Makefile target, then build/run this test and existing `waterfall_uv_wrap_test` and `parallax_speed_test` using the project toolchain. Only then take the exclusive physical-GPU ID37/ID40 held-camera and seam-crossing gate, logging actual SDL spans, selected path, destination and UVs, and checking tint/alpha/state restoration. Do not reuse fog PixelWobble's private mip cache.
