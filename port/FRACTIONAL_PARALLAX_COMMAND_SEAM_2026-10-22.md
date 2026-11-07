# Fractional parallax command-preparation seam — 2026-10-22

## Result

Added pure production-facing `port/linux/ParallaxUVCommand.h` and focused `port/tests/fractional_parallax_command_test.cpp`. `PrepareParallaxUVCommand` takes the already SDL-facing signed shift, unchanged destination and flips, full-texture/unrotated branch flags, and measured binding eligibility. It returns one of `NotApplicable`, `FloatQuad` (four float TL/TR/BR/BL UVs), or `LegacyInteger` (original shift/flip/destination retained for the existing nested integer-copy path). It does not bind textures, use GL/SDL, or modify renderer dispatch.

The CPU fake sink demonstrates exactly one float quad and zero integer callbacks at held-camera 1000.0→1000.5 and 1000.6→1000.7 for both 2048-wide forest IDs 37 (512 high) and 40 (677 high). U-left is .690000→.690345 inside old cut 1413 and .690414→.690483 across old cuts 1413→1414; corresponding deltas are .000345/.000069 and .70656/.141312 source texel. Signed flipped inputs use the −1 U span and opposite deltas. Destination is unchanged. Nonunit Y span, failed bind, non-GL renderer and unsupported NPOT repeat each yield one legacy command, zero float quads, and the old nested integer segments. Source-rect/rotation/zero-shift inputs are inapplicable.

## Gate and limits

Host `g++ -std=c++17 -Wall -Wextra -Werror -UNDEBUG` compilation and execution passed for the new command test and existing `fractional_parallax_uv_test.cpp`. No port build, GPU, Xorg or Docker was used. No SDLRenderer.cpp, fog files or shared status files were edited. This is preparation only: `SDLRenderer.cpp` still uses the old integer copies, so fractional runtime motion, SDL-bound spans, sampler-state restoration, and visual parity remain unproven. The new header includes the existing `port/linux/ParallaxUV.h`, which is still untracked in this checkout and must be included by its owning lane for a fresh-clone compile.

## Commit

`9154048` — Add pure fractional parallax command preparation seam. Pushed to `origin/feature/linux-renderer-foundation`; commit contains only the two new software files above. This report is the sole completion report for this lane and is intentionally not included in that software-only commit.
