# Forest-2 moving-camera ParallaxLayer placement implementation — 2026-10-24 01:23Z

## Result

Implemented the original `GameObject::Update` world-matrix-before-`ParallaxLayer::Update` follow-X order for all `followCameraX` ParallaxLayers, extending the previously ID59-only cached world-X path. In Forest-2 this corrects IDs 1/28/36 without changing current-camera UV phase, authored geometry/art/alpha, renderer, XML, or ID59 semantics. The source-backed command mismatch is closed; **original-Windows pixel parity remains unproven** because no synchronized original D3D10 frame/packet was available.

## CPU first-failure / correction

New production `LevelScene` draw-spy: `port/tests/forest2_parallax_prior_x_cpu_test.cpp`. Built with `g++ -O0 -std=c++17 -Iport/compat` plus `LevelScene.cpp` and TinyXML sources. Before the implementation it failed on first eligible Forest-2 draw: destination X `-21.6667` instead of authored-world `1123.66` (ID1). After implementation it passes initial authored placement, updates at camera X=100 then 108 (second draw uses world X=100), third eligible update at 108 (world X=108), two updates per draw, paused update and repeat draw; each step asserts all three destination widths/heights and current-camera X UV (`*.000059` / `*.000690`). This independently encodes the Windows update-before-follow predicate and catches draw-only substitutions.

Existing `waterfall_id59_prior_x_test.cpp` rebuilt and passed (authored initial X, moving/paused/multi-update/reload). Existing `parallax_speed_test.cpp` was amended to compare two *eligible-update settled* camera states, preserving its UV-speed assertion; it failed under the old draw-only schedule after the correction, then rebuilt/passed with valid update cadence. `git diff --check` passed. Docker `deathleak-linux-build:bookworm make -C port -j2` passed for old and new variants (existing compiler warnings only).

## Physical NVE7 same-state A/B

Old and new variants were separately built with the same current source tree and assets; the old scratch copy at `/tmp/deathleak-forest2-parallax-old` reverses only the parallax cached-X condition/draw selection, with no project source rollback. Binary SHA-256: old `3328f4069a17665c446bf3cf6b4e5d49680c29415dd3ba2b9797698638031bc2`; new `1c27850ea691e4003be7182ab49edbb5234fc25534789a5f3c3e63d57550128f`. Both were run sequentially via Docker with physical `DISPLAY=:0`, X11/OpenGL strict accelerated, `nouveau/NVE7`, `software=0`, GPU Sepia, zero missing assets, same Forest-2 scene/script `L:150`, 150 frames, fixed dt `0.0166667`, 1280×720 lossless BMP. Both final states match (Forest-2, player `(1285.99,3110.82)`, health 25, vx -480, grounded, two orbs; elapsed wall time differs). No forced camera/scene was used.

Full-composition old/new frame-150 RGB difference: **222,636/921,600** changed pixels, bbox `[0,33,1280,687)` half-open. With `DEATHLEAK_RENDER_ONLY_XML_IDS=1,28,36`, the same-state isolated A/B differs at **444,327 pixels**, bbox `[0,33,1280,603)`; full overdraw masks some layer differences. Counts use Pillow RGB `ImageChops.difference` and any-channel nonzero. Captures and logs: `port/build/private-forest2-parallax-lag-20261024t0118z/` (ignored build artifacts). This proves a material Linux pixel effect on a natural moving route, not that the new pixels match Windows or improve user-visible composition. The CPU spy, not GPU packet tracing, verifies exact destination/UV command arithmetic.

## Scope / release

Only `port/game/LevelScene.cpp`, `port/game/LevelScene.h`, `port/tests/forest2_parallax_prior_x_cpu_test.cpp`, `port/tests/parallax_speed_test.cpp`, and this report belong to this lane. Pre-existing dirty renderer, integration test, status, and other reports remain untouched. Physical GPU/build ownership is released after these captures. No original-Windows same-update D3D10 matrix/UV/pixel packet or visual acceptance is claimed.
