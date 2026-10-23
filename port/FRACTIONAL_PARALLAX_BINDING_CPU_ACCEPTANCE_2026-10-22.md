# Fractional parallax binding CPU acceptance — 2026-10-22

## Bounded result

Added `port/tests/fractional_parallax_binding_cpu_acceptance_test.cpp`, a standalone C++17 test of the existing `PrepareParallaxUVCommand` output against `ForEachUVWrapSegment`'s legacy copy mapping. At exact texel phases it compares every destination texel's X/Y source index (16×8) across all four flip combinations and positive/negative signed shifts, with destination geometry unchanged. This catches corner order, screen-edge flip orientation, wrap seam placement, and a second sign inversion on either axis. A separate quarter-texel change on both axes keeps all old integer source indices fixed while moving the command's float U and V by the expected amount. This is a concrete boundary not covered by the previous forest-X command test or isolated helper test.

Host verification: `c++ -std=c++17 -Wall -Wextra -Werror -UNDEBUG port/tests/fractional_parallax_binding_cpu_acceptance_test.cpp -o /tmp/fractional_parallax_binding_cpu_acceptance_test && /tmp/fractional_parallax_binding_cpu_acceptance_test` → `PASS: command UV to segment CPU acceptance (both axes, all flips)`.

## Renderer handoff boundary

`port/linux/SDLRenderer.cpp` still uses nested integer copies in `DrawSprite`'s UV-scroll arm (`:246-261`); this test does not prove that runtime code invokes the command seam. Binding owner should follow `FRACTIONAL_PARALLAX_BINDING_PATCH_SPEC_2026-10-22.md`: insert only after damage/wobble/foliage/repeat dispatch, require desktop `opengl` and current context, measure successful SDL bind spans and actual texture allocation/NPOT repeat support, then submit command TL/TR/BR/BL UVs unchanged. Preserve one legacy fallback for every rejected binding. Before and after the direct quad, save/restore unit-0 binding, active unit, program, matrix mode and projection/modelview stacks; restore wrap S/T and min/mag on the SDL-owned texture *before* unbind. Verify the next ordinary SDL copy for state leakage in the reserved physical/GPU lane.

No renderer source, shared status, build, SDL, GPU, Xorg or Docker was touched here. CPU parity at exact texel phases and retained fractional movement are proven; runtime binding and visual parity remain unproven.
