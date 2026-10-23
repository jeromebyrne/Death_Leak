# Fractional parallax Makefile/CPU gate — 2026-10-22

## Scope and result

Added `make -C port fractional_parallax_uv_test`, with explicit dependencies on `tests/fractional_parallax_uv_test.cpp`, `linux/ParallaxUV.h`, and `linux/UVWrap.h`. The application target now also lists `linux/ParallaxUV.h` as a header dependency for subsequent renderer integration. The CPU compile appends `-UNDEBUG` after caller-provided `CXXFLAGS`; the test has a preprocessor error if `NDEBUG` remains defined. No renderer or GPU path was changed.

The prior destination assertion compared two identical synthetic arrays. It now compares the calculated 2/3-scale synthetic rectangle against independent expected ID37 (2048×512 → 1365.333333×341.333333) and ID40 (2048×677 → 1365.333333×451.333333) values, and rejects other test heights. This verifies only the test's authored geometry calculation, **not** an actual `DrawSprite` destination or visual parity.

## CPU-only verification

- `make -C port fractional_parallax_uv_test` — PASS: fractional parallax CPU corners, signed phase, seam, span gate and dispatch.
- `make -C port -B fractional_parallax_uv_test CXXFLAGS='-O2 -std=c++17 -Wall -Wextra -Wpedantic -DNDEBUG'` — PASS; command visibly compiles with `-DNDEBUG -UNDEBUG`, so assertions remain enabled despite a release-flag override.
- Re-ran `make -C port fractional_parallax_uv_test` after the final test edit — PASS.

No Docker build, SDL application build, GPU/Xorg/display run, or renderer integration test was performed. `linux/ParallaxUV.h` and the CPU test were initially untracked, and ownership of the helper remains outside this lane; a scoped commit here cannot by itself make a remote checkout's new target buildable until the helper is committed by its owner.
