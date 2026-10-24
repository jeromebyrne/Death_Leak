# Forest-2 ID37 four-leaf ParticleSpray implementation / GPU gate — 2026-10-24 01:02Z

## Disposition

Implemented the source-backed **Forest-2 ID37 only** loader admission and whole-emitter draw gate. Focused CPU fixture and full Docker build pass. A controlled physical NVE7 A/B shows a small conditional pixel effect under a forced draw camera; the authored initial camera shows no pixel delta. This is command parity, **not** proof of a large natural-route visual improvement or original-Windows pixel parity.

## Source and scope

`forest_exploration_2.xml` ID37 authors four `oak_leaf.png` particles, not an emitter-sized texture quad. Windows loads this `ParticleSpray` and admits its draw on emitter bounds before rendering particles. `port/game/LevelScene.cpp` previously admitted only Forest-1 ID47 and waterfall-top IDs49–51. This patch adds precisely `(forest_exploration_2.xml,37)` to that allowlist and applies the existing scaled `Forest47EmitterInView` predicate to ID37 as well as ID47. Common particle physics, RNG, renderer, XML for other scenes, and dirty unrelated files are untouched. ID37 is at depth 1750 in the existing painter merge.

## Verification

- Docker image `deathleak-linux-build:bookworm`, assertions enabled: new `forest2_id37_emitter_boundary_cpu_test_20261024t0059z.cpp` first **failed as expected** at its leaf-texture loader assertion on unpatched source. With both changes it printed `PASS: Forest-2 ID37 authored admission and emitter-boundary draw gate`. It verifies XML asset/ID/depth/bounds/four staggered lifetimes, a visible leaf with off-emitter and exact-contact rejection, in-emitter admission, flipped U, and 80-world-unit leaf scaling to 53⅓ pixels.
- Existing Forest-1 `forest47_emitter_draw_cpu_test.cpp` rebuilt and exited 0. Current-tree `make -C port -j2` in Docker exited 0 (pre-existing tinyxml warnings only).
- GPU packet path: `port/build/private-forest2-id37-gpu-20261024t0102z/`. An isolated source copy `/tmp/deathleak-id37-gpu-0102` used the current `LevelScene.cpp` and inherited an unrelated opt-in camera-Y diagnostic hook from the prior ID47 scratch tree. No diagnostic hook was put in the project source. A/B uses the **same isolated binary** and `DEATHLEAK_RENDER_HIDE_XML_IDS=37` for the hidden control, `forest_exploration_2.xml`, script `.:1`, frame 0, fixed dt 0.0166667, 1280×720. Both logs report strict accelerated X11/OpenGL, `nouveau`/`NVE7`, initialized GPU Sepia.fx, 62 loaded scene sprites, zero missing assets.
- Forced draw-camera Y=4300 (the authored initial camera is Y=3400): shown vs hidden differs in **289 RGB pixels** (any channel), bbox `[630,356,869,462)`. `difference.png` and lossless BMPs are saved. The 148-pixel grayscale difference count is not used because converting RGB differences to luminance drops some changed pixels.
- Natural initial-camera A/B, same binary and frame, differs in **0 pixels**; BMP RGBs are identical. At camera Y=3400 the initial leaves around emitter Y=4345 are outside the viewport. The forced-camera result proves rendering capability and a conditional effect, not route relevance. No matched D3D10 Forest-2 frame, aligned Windows RNG state, or Jerome visual acceptance exists.

## Handoff

Scoped software is ready to commit/push: `port/game/LevelScene.cpp`, the new focused fixture, and this report only. All pre-existing modified checkpoint, renderer and test files remain unstaged. Physical GPU/build lane is released.
