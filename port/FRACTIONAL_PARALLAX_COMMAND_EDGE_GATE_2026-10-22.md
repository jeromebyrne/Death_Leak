# Fractional parallax command edge gate — 2026-10-22

Scope: CPU-only continuation at `9154048`. The only code edit is `port/tests/fractional_parallax_command_test.cpp`; no renderer, production helper, shared status, build, Xorg, Docker, or GPU work.

## Evidence

- Added rejected-binding vectors for non-GL backend, failed bind, zero X/Y span, negative X/Y span, nonunit X/Y span, infinite X span, and NaN Y span. Each expects `LegacyInteger`, unchanged four destination fields, both signed shifts and both flip flags, zero float quads, and four nested integer copies/eight segments for finite shifts.
- Added `+inf`, `-inf`, and NaN in each shift axis with otherwise eligible binding. Each expects `LegacyInteger` and preserved destination/shift/flip. These commands are deliberately **not submitted** to `UVWrap`: its float-to-int conversion is undefined for nonfinite shifts. This test proves command preparation only, not safe execution of a nonfinite legacy copy.
- Existing ID37/ID40 held-cut/crossing, both-flip, and exclusion vectors remain active.
- Standalone verification passed: `g++ -std=c++17 -Wall -Wextra -Werror -UNDEBUG port/tests/fractional_parallax_command_test.cpp -o /tmp/fractional_parallax_command_edge_gate_20261022 && /tmp/fractional_parallax_command_edge_gate_20261022` printed `PASS: fractional parallax command CPU seam`. `git diff --check -- port/tests/fractional_parallax_command_test.cpp` passed.

## Limit

`SDLRenderer.cpp` is not wired to this command seam at this checkpoint. This gate does not prove renderer path selection, actual SDL bind spans, GL state restoration, GPU visual parity, or nonfinite legacy-copy safety. `port/linux/ParallaxUV.h` remains an untracked dependency in this checkout, so a fresh clone needs that header supplied by its owning lane before the standalone test compiles.
