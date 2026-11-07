# Fractional parallax live bind — 2026-10-22

## Bounded result

Implemented the full-texture, zero-rotation UV-scroll renderer arm in `port/linux/SDLRenderer.cpp` using `PrepareParallaxUVCommand`. Eligible draws emit one GL quad with continuous signed SDL-facing UVs; ineligible or pre-draw-failed binds take the existing nested SDL integer-copy fallback once. The effect priority, source-rect/rotation/zero-scroll ordinary arms, existing fog mip upload, and the dirty controller override were not changed. `port/Makefile` now tracks `ParallaxUVCommand.h` as a renderer prerequisite.

Eligibility requires finite shifts and positive finite destination geometry, positive SDL texture dimensions, the selected `opengl` renderer and current context, successful SDL bind, nonzero bound GL texture, successful positive level-0 GL allocation queries, unit X/Y bind spans, and full NPOT repeat support for an NPOT allocation. GL >= 2.0 or the exact ARB NPOT extension establishes support. Forest ID40 with a subunit Y span remains on the integer path. One-time per-texture diagnostics report logical/allocation dimensions, spans, NPOT support and chosen path after a GL bind attempt.

The float draw flushes SDL first, snapshots program/matrix mode/active unit/unit-0 binding and GL attributes, snapshots/restores all four bound-object sampler parameters, and restores matrices, program, binding and active unit. It uses texture-0 fixed-function modulation, tint/opacity once, source-over blending, no depth, and the 1280×720 logical projection. No mip levels or private wobble texture are modified. This is continuous-phase binding, not a claim of exact Windows mip-linear parity.

## Verification

- Assertion-enabled CPU tests passed: `fractional_parallax_uv`, `fractional_parallax_command`, `fractional_parallax_binding_cpu_acceptance`, and `fractional_parallax_nonfinite_cpu_gate`. These exercise command selection, all flips, unchanged destination, rejected spans/NPOT, one-quad vs legacy-copy sink, and NaN/±inf at the actual integer-wrap sink.
- Docker SDL software `waterfall_uv_wrap` passed; source camera/phase `parallax_speed` passed.
- Forced Docker production build passed against the final edited source: `docker run --rm -u "$(id -u):$(id -g)" -v "$PWD:/src" -w /src deathleak-linux-build:bookworm make -C port -B -j2 all`. Only existing TinyXML warnings. `git diff --check -- port/linux/SDLRenderer.cpp port/Makefile` passed.
- No physical GPU/display capture or visual parity claim was made. A true live GL failure-injection/state-restoration seam was not run; the CPU command seam and source inspection do not replace that acceptance gate. The renderer diagnostics and matched-frame comparison remain for the separately owned GPU lane.

## Integration hold

No commit or push: `port/linux/ParallaxUV.h` is an **untracked prerequisite owned outside this scoped lane**; committing only these two tracked code files would leave a clean checkout unbuildable. Preserve and stage that helper in a coordinated integration commit, then perform the live GL seam/GPU acceptance before claiming visual fidelity. Existing unrelated dirty files were left untouched.
