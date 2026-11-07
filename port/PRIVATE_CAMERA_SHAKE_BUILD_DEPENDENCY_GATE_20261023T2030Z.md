# CameraShake build-dependency repair — 2026-10-23

## Result

Added `game/CameraShake.h` to the Linux application target's explicit prerequisites in `port/Makefile`. This closes the header-only stale-binary risk identified in `PRIVATE_POST_CLIP_REGRESSION_GATE_20261023T2028Z.md`; no runtime source or capture path changed.

## Verification

- Clean isolated Docker compilation: `docker run --rm -v "$PWD":/src:ro -w /src/port deathleak-linux-build:bookworm make TARGET=/tmp/deathleak-camera-shake-build-gate all` exited 0. The source mount was read-only and the target was inside the disposable container; no shared build output was touched. Only existing TinyXML compiler warnings appeared.
- Controlled incremental freshness in a separate container: a newly touched temporary target made `make -q ... all` exit 0; after a one-second interval, `make -q -W game/CameraShake.h ... all` exited 1 (rebuild needed). `make -n -W ...` contained the `g++ ... linux/main.cpp` compile recipe. Result: PASS, header-only change marks the app stale.
- `git diff --check -- port/Makefile` passed. The scoped commit contains only `port/Makefile`; unrelated tracked and untracked dirty files were preserved.

## Delivery

Commit `8480942f64894b34857781140ff04a812573743d` (`Fix CameraShake header dependency in Linux build`) pushed to `origin/feature/linux-renderer-foundation` (`6dfc65b..8480942`).
