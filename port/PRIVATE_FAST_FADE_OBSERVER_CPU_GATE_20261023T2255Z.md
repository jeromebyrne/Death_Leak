# Fast Forest→Waterfall fade CPU observer gate — 2026-10-23 22:55Z

**PASS: connected, no-pixel CPU observer cadence.** This closes the nonvisual loading-fade observer gap left by the 426-frame software-rendered timeout in `PRIVATE_CURRENT_NONVISUAL_RELEASE_RECOVERY_20261023T2249Z.md`. It does **not** claim a pixel/GPU presentation pass or physical keyboard acceptance.

## Isolated implementation

- Added `tests/private_fast_fade_observer_cpu_renderer_20261023t2255z.h`: a test-only renderer with real asset-existence checks and stable nonzero texture IDs, but no pixel allocation, SDL video, Xorg, GL, or frame composition. Compiled into `main.cpp` using `-include` and **without** `SDLRenderer.cpp`, leaving production source unchanged. The existing `DEATHLEAK_TEST_LOADING_FADE_OBSERVER` instrumentation stays on the actual main-loop update/draw path.
- Added `tests/private_fast_fade_observer_cpu_gate_20261023t2255z.py`: a repeatable read-only source bind, isolated `/tmp` build/run, `--network none`, 180-second compile timeout, 40-second run timeout, and existing `private_loading_fade_cpu_regression_20261023.py` assertions. It uses the already-local `deathleak-linux-build:bookworm` image but no shared Docker build/output. Audio uses SDL's dummy audio driver; the CPU renderer does not prove image decoding or pixels.
- Run: `PYTHONDONTWRITEBYTECODE=1 python3 port/tests/private_fast_fade_observer_cpu_gate_20261023t2255z.py` from repository root.

## Observed result

The repeatable gate exited 0: `PASS connected loading fade: hold-exit frame 414, first gameplay FadeOut frame 415, retirement frame 424`. The direct run exited 0 in approximately 0.033 seconds of reported simulation runtime after the isolated compile. Forest trigger occurred at frame 398; Waterfall installed exactly once at frame 411. FadeIn remained opaque at frame 410 and paused gameplay/weather/scene/player for frames 399–411. OpaqueHold remained through frame 413. Frame 414 exited hold with alpha 255 and gameplay still paused. Frame 415 resumed all four update/visual gates under alpha 170. Frame 423 reached alpha 0; frame 424 retired the splash (`alpha=-1`) with gameplay active. Existing regression assertions checked these strict-boundary and single-install contracts.

`git diff --check` passed before report creation. This test is a no-render production-loop timing gate, not a visual compositing or physical-display release gate. No production patch is indicated. Existing tracked dirt, other owners' untracked files, shared build output, checkpoint/status files, and index were not changed.
