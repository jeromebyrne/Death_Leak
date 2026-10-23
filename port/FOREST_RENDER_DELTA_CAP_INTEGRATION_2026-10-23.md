# Forest rendered-update delta cap integration — 2026-10-23

## Result

Integrated the source-grounded 1/60-second maximum update timestep in `port/linux/main.cpp`. Original `Game/src/program.cpp:115-117,149-175` waits for at least its 1/60 target, caps an overlong elapsed interval to that target before `Update` and `Render`, then commits the same modified delta to `SetLastUpdateDelta` after `Render`. Linux previously capped the elapsed/fixed override at 0.05 s and committed that larger value to `precedingRenderedModifiedDelta` after `EndFrame`. The production loop now caps `dt` at `1.f/60.f` before its existing consumers; the already integrated post-render assignment remains unchanged. No time modifier exists in this port yet.

This closes the bounded slow-rendered-frame discrepancy described by `FOREST_ANIMATION_CLOCK_CURRENT_HEAD_QA_2026-10-23.md`: after a 50 ms iteration, the next ID2/ID68 authored animation update now receives one capped prior rendered delta, not 50 ms. The same cap also applies to current-delta physics, combat, weather, shader, and timers, as in the original update loop. At 50 ms/frame the simulation advances at most about 16.67 ms per update (one third of wall elapsed), rather than 50 ms; this is intentional source behavior, not wall-time catch-up. The existing `--fixed-dt` test override is capped as well. This change does not add the Windows minimum-frame wait/present cadence, damage time modifier, or multiple catch-up steps, and makes no pixel-parity claim.

## Verification and impact gate

- Docker production build: `docker run --rm -v "$PWD:/src" -w /src deathleak-linux-build:bookworm make -C port -B -j1 all` **PASS**; only existing TinyXML warnings.
- New CPU-only production-loop regression `python3 port/tests/render_delta_cap_cpu_test.py`: **PASS**. With the same 12-frame rightward forest script, a fixed 50 ms timestep produces exactly the 1/60-second simulation summary (elapsed wall-time field excluded); an 8 ms subtarget run has a distinct player x, demonstrating that the cap does not force every update to 1/60. It uses the test-only SDL software renderer solely to initialize the headless simulation; it performs no capture/display review.
- Adjacent authored-XML ID2/ID68 prior-delta command regression: **PASS** (`forest_preceding_delta_cpu_test.cpp`). A pre-existing 340-frame forest-left route with the common `--fixed-dt 0.0166667` still enters `grass_exploration_3.xml` at frame 300 under the exact cap.
- Scoped `git diff --check`: **PASS**. No shared status, GPU, SDL renderer, integration script, strafe fixture, XML, or asset files were edited by this lane.

The test verifies deterministic simulation and command-path behavior; it does not measure physical GPU behavior or establish matched Windows/Linux video. Existing broader integration tests using rounded 0.0166667 may differ at rare floating-point thresholds after this exact cap, and should be evaluated by their owners if such a failure occurs. Future implementation of damage slowdown should commit the actual modified update delta, not an unmodified wall interval, to the existing prior-render slot.
