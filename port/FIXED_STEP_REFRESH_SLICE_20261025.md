# Fixed-step gameplay / refresh decoupling — first Linux vertical slice

Status: scoped first slice accepted for gameplay cadence; high-refresh presentation and release acceptance remain open. Base `536a9a4` on `feature/linux-renderer-foundation`.

## Implementation

`port/linux/FixedStepPacer.h` accumulates monotonic wall time in 1/60-second quanta. The live main loop consumes one fixed update per ordinal, waits briefly when ahead, and omits intermediate presents when catching up. Scene visuals and pixel-wobble shader phase still advance on every update. Debt is bounded to 120 steps (two seconds); any dropped quanta are reported. `--fixed-dt` retains the prior one-update/one-present deterministic path. `--test-refresh-hz` is restricted to scripted `--simulate-only` routes and injects 30–240-Hz synthetic display slots into the *same* pacer; it never claims an actual monitor mode. The Makefile tracks the new header.

This is not interpolation: it limits distinct presentations to the 60-Hz simulation cadence and presents fewer when the GPU is slower. Input is sampled once per update. There is no >60-FPS rendering claim.

## Evidence and rejected intermediate choices

- Simulated clock 10-second slot tests at 30/60/75/120/144/165/240 Hz each produced 601 fixed updates, including the initial update, and no dropped quanta. A first nanosecond boundary implementation yielded only 405 presents at nominal 60 Hz despite 601 updates; rejected and corrected with a 100-ns boundary tolerance.
- Actual `--simulate-only --script R:180 --frames 180 --test-refresh-hz` game routes at all seven rates ended at player `(1368.99,3107.30)`, the same as the fixed-dt route, with zero dropped quanta. Synthetic slot counts: 90/179/224/358/430/493/716. These are CPU/gameplay timing tests, not high-Hz display tests.
- The first live candidate capped debt at eight steps and dropped seven during startup. Rejected that policy; the final 120-step cap removed observed loss. The initial A/B/A using the previous display-slice executable also proved unsuitable for exact full-frame parity because the 11-FPS film overlay is process-uptime driven. An exact-HEAD baseline was rebuilt with the same clean renderer as candidate.
- Serial physical Xorg `:0` / SDL `x11` OpenGL accelerated=1 software=0 vsync=1 / nouveau NVE7, fixed-dt Forest→Waterfall-top `R:800,L:1200` 425-update A/B/A: frame-424 **pre-HUD** 1280×720 BMP files were byte-identical in all three arms, SHA-256 `55b275c6fe7867088073525aa1a6ca3fadc4f635ea0c50b6e9f542d68f581ece`. Gameplay endpoint was `(-2139.01,3114.7)` in all arms. Short two-update full-frame A/B also matched exactly; the second control differed solely in the existing time-driven film overlay, so full post-HUD repeat parity is not claimed.
- Serial physical live `R:180` A/B/A, 180 update ordinals: controls elapsed 5.070/4.227 s versus candidate 3.044 s. Candidate endpoint `(1368.99,3107.30)` matched fixed-step; controls were `(1367.01,3107.29)` and `(1365.51,3107.28)` under variable capped dt. Candidate reported zero dropped quanta. Candidate had 35 measured presents after 120-update warmup versus 60 per control, consistent with catch-up render omission. Per-present p50 was 30.68/29.94/21.66 ms in A/B/A, with physical GPU variability; this is a game-speed gain, not a measured FPS gain. Fixed-dt Waterfall A/B/A frame p50/p95 were 20.73/22.85, 20.70/22.88, 21.54/33.38 ms (third control drifted), likewise not an optimization claim.

Reproduction artifacts: ignored `port/build/performance-fixedstep-20261025/` contains frozen clean-HEAD renderer, exact baseline and candidate binaries, build logs, rate harness and scripts, six physical route logs/captures, pre-HUD atomic packets and synthetic-rate logs. Both baseline and candidate were compiled in `deathleak-linux-build:bookworm`; renderer sources were frozen from clean HEAD, not the pre-existing unstaged raft-mip experiment.

## Remaining distinct task

Split simulation from drawing so the display can present at supported >60-Hz modes, with interpolation or proven held-state rendering; give camera shake, HUD film, wall-clock visual/audio paths explicit update/presentation clocks. Validate full post-HUD exact pixels at synchronized simulation *and film* phase and physical high-Hz mode only if an actual display offers it. Current machine has 1440×900@60 current mode and no 4K/8K advertised mode per the preceding display slice. This slice does not establish sustained 60 FPS, memory/VRAM/thermal acceptance, 8K operation, or release readiness.
