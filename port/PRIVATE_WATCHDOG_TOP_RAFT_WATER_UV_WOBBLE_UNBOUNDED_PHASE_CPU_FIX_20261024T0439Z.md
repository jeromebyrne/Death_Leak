# Waterfall-top wobble auto-scroll V phase — scoped CPU fix

## Result

Implemented source-parity correction in `port/game/LevelScene.cpp`: auto-scrolling pixel-wobble sheets retain the full accumulating V phase, while ordinary sheets continue floor-wrapping their phase. This targets Waterfall-top IDs 13/14 (`effectpixelwobble`) without changing ordinary IDs 6/7/31 or any renderer/shader code.

## Source contract

`Game/src/GameObjects/ParallaxLayer.cpp:114-150` adds the scaled, prior-render Y delta to stored vertex V without modulo. `Game/ShaderFiles/BasicLightTexture_PixelWobble.fx:55-65` applies a U-dependent V perturbation, then computes U using `sin(V * Time * SpeedMod)` before texture wrap. The port previously floored away the integer part of V at `LevelScene.cpp:1450`; that is sampler-equivalent for ordinary sheets but not for the wobble's second sine. The correction leaves cadence, speed scaling, culling order, effect clock, and ordinary wrap unchanged.

## CPU verification

- New `port/tests/waterfall_wobble_unbounded_phase_cpu_test.cpp` loads the authored Waterfall-top XML and captures actual `LevelScene::Draw` submissions. Across 480 draws it checks both wobble IDs 13/14 maintain source-style unbounded V through integer phase, checks the production `PixelWobbleVertexUV` helper receives that phase, and demonstrates a nondegenerate second-sine U difference from floor-reduced V. Ordinary ID6 stays floor-wrapped after its own integer boundary. PASS.
- Existing `port/tests/waterfall_autoscroll_test.cpp` PASS: prior-render delta cadence, modal pause, repeated draws, and reset remain intact.
- `git diff --check` passed for the scoped source patch. CPU only; no GPU/Xorg, same-state RGB comparison, or Windows binary capture claimed.

Only `port/game/LevelScene.cpp`, the new focused CPU test, and this report were authored in this lane. Shared dirty files and bottom-present946, SDLRenderer, Sea-2, and checkpoint/status files were untouched. Lane released after scoped commit/push.
