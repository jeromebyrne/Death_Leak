# ScrollingSprite motion/fade checkpoint — 2026-10-22

## Scope and source evidence

Implemented the original `ScrollingSprite::Update` movement and alpha cycle in `port/game/LevelScene.{h,cpp}` for **the `ScrollingSprite` XML kind**, not for forest-only IDs. `Game/src/GameObjects/ScrollingSprite.cpp:19-61` supplies elapsed interpolation, fade-in/out, and the reset/skip-frame endpoint; `DrawableObject.cpp:213-219` captures original XML alpha. The port parses all seven `scroll_properties` attributes and retains per-sprite original alpha. It updates only while the sprite is updateable and in the existing 4000×4000 manager-style camera zone (or `always_update`), then draws the resulting XY/alpha through the existing renderer. Existing XML-ID diagnostic filtering is unchanged.

Forest-1 IDs 119/122 (`forest_exploration_1.xml:1631-1662`) now move +1000/−750 X over 45/70 s and fade from authored 0.5/0.6 over 6 s. At a 1 s update, expected effective sprite alpha before PNG texel alpha is 0.0833/0.1000; at 5 s it is 0.4167/0.5000. At 6 s it reaches 0.5/0.6. The original reset tick retains the preceding drawn position/alpha, then restarts next update.

## Verification

- Focused `port/tests/scrolling_sprite_test.cpp` compiled with host `g++` and passed: ID 119/122 1/5/6 s alpha, X drift, update-zone pause, fade-out, endpoint skip, and restart.
- Existing `port/tests/occlusion_fade_test.cpp` compiled and passed, confirming the separate tree-occlusion path still behaves as before.
- `git diff --check` passed for the implementation. Native full build was unavailable because host `pkg-config` lacks `sdl2`, `libpng`, and `gl`. No Docker build or GPU/capture was run here; that lane is owned by the video worker.

## Remaining fidelity gap

`effectpixelwobble` on both fog sprites is still not represented in the port renderer. The source `BasicLightTexture_PixelWobble.fx` perturbs texture UVs using the XML intensity 0.07 and speed 0.3; this change intentionally leaves that renderer/shader work separate. No same-frame baseline/only/hide images or matched Windows frame were captured in this lane, so this is a source-behavior correction, **not** a claim of visual or pixel parity or proof that the fog alone causes the reported horizontal seam. The video worker can use IDs 119/122 at deterministic fresh-start 1/5/>6 s frames to evaluate their contribution.
