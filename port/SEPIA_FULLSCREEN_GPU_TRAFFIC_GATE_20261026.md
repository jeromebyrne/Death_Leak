# Sepia fullscreen GPU pass and scene-target traffic gate — no safe candidate

Starting point: detached clean remote `35e4e4046890710f7758cdea8417372ece2bf035`. The dirty primary source tree (seven unrelated tracked paths, including raft-mip edits in `port/linux/SDLRenderer.cpp`) was not used for the build or changed. Test-only timer/counters were isolated in `/tmp/deathleak-sepia-gpu-20261026`; no renderer patch is proposed.

## Physical method and result

The isolated renderer wraps only the `glBegin(GL_TRIANGLE_STRIP)`/`glEnd()` Sepia quad with `GL_TIME_ELAPSED` and counts scene-target begins, output-target switches, `SDL_GL_BindTexture` calls, framebuffer copies, and Sepia draws. Two independent serial fixed-dt 180-update runs per route used physical Xorg `:0`, SDL x11/OpenGL strict accelerated=1/software=0/vsync=1, Mesa nouveau/NVE7, actual/logical 1280×720. No input kept the Waterfall route in Waterfall-top and Forest in `forest_exploration_1`. Every run reached the same route-specific gameplay endpoint: Waterfall `(818.985,2933.64)`, one orb, 25 health, zero shots; Forest `(-43.0146,3206.9)`, one orb, 25 health, zero shots. Each run had exactly 180 scene-target begins, 180 output-target switches, 180 scene-texture binds, **zero** framebuffer copies, and 180 Sepia fullscreen draws.

| Route | GPU Sepia quad elapsed, runs 1/2 | Per frame, runs 1/2 |
| --- | ---: | ---: |
| Waterfall-top | 207.269 / 211.052 ms | 1.151 / 1.173 ms |
| Forest | 219.346 / 214.288 ms | 1.219 / 1.190 ms |

These are measured GPU quad/fragment spans, not whole-frame percentages or projected removability. Immediate `GL_QUERY_RESULT` reads serialize the GPU; timer-on runs cannot be compared to query-off frame latency as a speedup. Separate serial query-off 300-update controls (120 warmup, 180 samples) gave Waterfall p50/p95/p99 21.815/22.012/22.178 and 21.825/22.055/22.170 ms; Forest 21.792/21.875/22.440 and 21.708/21.811/22.419 ms. These are **A/A baseline controls**, not candidate A/B/A results.

The scene target is 1280×720 RGBA8 (3,686,400 bytes). The pass logically samples one target pixel and writes one output pixel per fullscreen pixel, approximately 7.373 MB/frame of logical Sepia read+write; scene-target rendering itself writes additional pixels. This is an operation-size estimate, **not measured DRAM/VRAM bus traffic** (cache, overdraw and compression are unknown). The measured traffic facts are the target transitions/binds/copies/draws above.

## Candidate disposition

**No truly avoidable per-frame pass/target operation was established; no candidate accepted or shipped.** The existing covered-output clear was already removed in `73b7074`; this gate measured the remaining path. The accelerated route has zero framebuffer copies. The target switch and texture bind are required to sample the completed scene; its Sepia transform must run after scene compositing but before unprocessed HUD. Removing the Sepia quad changes authored color. Applying Sepia independently to scene sprites is not algebraically equivalent to transforming the composited scene: alpha blending, the source shader's nonlinear red-excess `step` branch, and HUD painter boundary forbid that fusion without a separately proven design. SDL's bind/unbind pair is not a proven redundant GL traffic operation. No measured safe clear, copy, or extra target pass remains in this span.

There is therefore **no B arm**. Synchronized exact full-frame candidate pixels, serial physical A/B/A GL/latency/traffic, and candidate gameplay endpoint are **not met** and are not inferred from the baseline controls. Native-clock captured BMPs are evidence of the physical route only, not a pixel-parity proof; the game's film clock can differ between control runs. Do not claim 60 FPS, 8K/high-Hz, broad atlas, thermal/VRAM, or release acceptance.

Evidence: isolated test-only renderer diff, build log, four query-on route logs and BMPs, and four query-off profile logs copied to ignored `port/build/sepia-span-20261026/`. Next distinct target: measure a non-Sepia expensive operation with an actual removable boundary, such as active-scene SDL renderer flush/target-transition serialization or large ordinary-sprite overdraw; a new candidate must clear synchronized full-frame and serial physical A/B/A gates before production code is changed.
