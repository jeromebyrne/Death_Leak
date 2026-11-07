# Waterfall-top V autoscroll: implementation handoff (read-only source audit, 2026-10-22)

Scope: independent follow-up to `WATERFALL_TOP_VISUAL_GAP_2026-10-22.md`. No code, build, GPU capture, or shared checkpoint changed for this audit.

## Exact legacy rule and clock ownership

- `Game/src/GameObjects/ParallaxLayer.cpp:114-155`: **Draw**, not `Update`, computes `percentDelta = Timing::GetLastUpdateDelta() / Timing::GetTargetDelta()`, then `yShift = prior vertex[0].TexCoord.y + (m_autoScrollYSpeed * 0.001f) * percentDelta` when `m_autoScrollY`. It assigns `(yShift, yShift, yShift+1, yShift+1)` to the four V coordinates, marks the vertex buffer dirty, and draws. Autoscroll takes precedence over camera-Y parallax if both are enabled. The bump-effect vertices have an equivalent separate path. This is accumulated phase, not `elapsedTime * speed` recomputed from zero each draw.
- `Game/src/GameObjects/ParallaxLayer.cpp:26-39,167-175` reads `autoscrolly` and `autoscrollyspeed` from XML; its `Update` only follows camera X/Y. Constructor sets `mAlwaysUpdate=true`. `Game/src/GameObjects/GameObjectManager.cpp:205-247` visits always-update objects even outside the normal update zone, but the UV increment occurs in `Draw`; do not couple phase to camera motion or culling of the authored waterfall sheets.
- `Game/src/program.cpp:115-117,149-178` sets target delta `1/60`, caps current delta at target, runs update, then renders, **then** stores `LastUpdateDelta(delta * TimeModifier)`. Thus Draw sees the preceding completed update's delta (initially zero), including time modifier; a port using the current `dt` in `UpdateVisuals` is one frame ahead on delta changes but has the same steady-state rate. Document/choose this small cadence distinction deliberately, particularly if exact first-frame/slow-motion matching is required. Do not advance per `Draw` in the port: diagnostic re-draws would change state.
- Legacy `ParallaxLayer::Scale` multiplies positive Y speed by `y` (`ParallaxLayer.cpp:223-232`); level loading scales authored objects by backbuffer height/1080 (`GameObjectManager.cpp:451-463`). At the 1280x720 target that is **2/3**. The faithful normalized-V increment at steady 60 Hz is therefore XML speed × (2/3) × .001 per frame: 7→.00466667, 9→.006, 11→.00733333. Per second: .28, .36, .44 UV cycles respectively. At a different target height use its height/1080 instead; if the port intentionally operates in unscaled authored coordinates, explicitly reconcile that choice against legacy output.

## Authored waterfall layers

All below are `waterfall_4.png` `ParallaxLayer`s in `Game/XmlFiles/levels/waterfall_top.xml`; each has `autoscrolly=true`, `autoscrollx=false`, `followcamy=false`, `followcamx=false`, and camera-Y multiplier zero. XML `alpha` is listed; depth affects painter order and must be retained.

| ID | XML lines | Y speed | Depth | Alpha |
|---|---:|---:|---|---:|
| 6 | 109-131 | 7 | kGroundBack | .5 |
| 7 | 133-155 | 11 | kGroundFront | .2 |
| 10 | 187-209 | 9 | kFarForeground | .2 |
| 13 | 261-283 | 11 | kGroundBack | .3 |
| 14 | 285-307 | 11 | kGroundBack | .3 |
| 15 | 309-331 | 11 | kGroundFront | .2 |
| 16 | 333-355 | 9 | kFarForeground | .2 |
| 31 | 563-585 | 7 | kFarForeground | .2 |

## Port insertion points and UV caveat

- `port/game/LevelScene.h:17-58`: `SceneSprite` is the per-layer owner; add XML autoscroll-Y flag/speed and a per-instance accumulated V phase there. `LevelScene.cpp:357-364` already reads follow-camera/parallax but omits autoscroll. Initialize/reset naturally on scene load/level transition. Keep authored quad XY, depth, alpha and asset unchanged.
- `LevelScene.cpp:656-735` is the mutable visual update path; `port/linux/main.cpp:447-450,1071-1075` supplies `dt` and calls it once before draw. Advance each enabled layer once per visual frame using the source-normalized rule, e.g. `phase += speed * (720.f/1080.f) * .001f * (dt / (1.f/60.f))` for this fixed 1280x720 target. Decide explicitly whether to emulate the legacy previous-delta one-frame lag; do not accidentally increment during each `Draw` (which is `const` at `LevelScene.cpp:835`). Maintain a bounded/fractional phase if desired, but compare modulo 1 in tests.
- `LevelScene.cpp:902-912` presently emits `draw.uvShiftY = -cameraY * cameraParallaxY` and sends the whole quad through `render::Sprite::uvShiftY` (`Renderer.h:51`); for these eight sheets that shift is zero. Add phase to this existing parallax path with the sign derived from actual top/bottom texture orientation, not by translating the quad. Do not alter X parallax or `repeatX` behavior.
- **Renderer risk requiring a targeted test:** `port/linux/SDLRenderer.cpp:213-239` enters a seam-splitting path for nonzero UV shift, splits source at wrap and calls `SDL_RenderCopyExF` on each tile with the sprite's `SDL_FLIP_VERTICAL` flag. The port sets ordinary authored sprites' `flipY = !object.flipY` at `LevelScene.cpp:919-928`, so these sheets can enter the vertical-flip-plus-wrap combination for the first time. Flipping each source tile separately does not automatically implement a whole-quad flipped, wrapped UV: tile ordering and seam placement must be checked/corrected against the four-vertex D3D mapping. A numerically correct `uvShiftY` can otherwise produce a stationary or reversed pattern/discontinuity. Preserve alpha across all split tiles and avoid zero-width seams at integer phases.

## Small independent regression gate (future implementation worker)

Extend the existing capture-renderer pattern in `port/tests/parallax_speed_test.cpp` (or add a waterfall-only sibling). Load the real `waterfall_top.xml`, filter IDs `6,7,10,13,14,15,16,31`, call `UpdateVisuals` at fixed `dt=1/60`, and record `Draw` outputs at fixed camera/player coordinates before and after 1 and 60 updates. Assert: all eight retained; per-ID phase modulo 1 matches 7/9/11 × 2/3 × .001 × tick count (within float tolerance); rate ratios 7:9:11; same destination/depth/tint/opacity at fixed camera; no phase change from repeated `Draw` without update; and changing camera-Y does not alter these zero-multiplier phases. Include a non-waterfall camera-parallax ID as a guard against breaking existing UV shifts. Add a tiny asymmetric, vertically labeled/checker texture renderer test at phases 0, .25, .75 and just across 1.0 with `flipY=true` to verify direction, seam continuity, and tile alpha: sprite capture alone cannot prove SDL's texture-copy mapping.

## GPU visual acceptance recipe (after renderer lane is free)

Use a **real GPU/SDL** renderer, not the software escape hatch. From `source/`, run the port in active `--play Game/XmlFiles/levels/waterfall_top.xml` at `--fixed-dt 0.016666667`; choose a saved position/short input script or an existing active route that holds the camera nearly fixed on the broad shaft/bridges/orbs. The CLI captures only the final frame of a run (`port/linux/main.cpp:1183-1186`), so make two otherwise identical runs with the same starting state/script and `--frames N --capture early.bmp` and `--frames N+60 --capture late.bmp`. `--camera-script` cannot be used here: it requires `--simulate-only --play` and simulation skips `UpdateVisuals`/render (`main.cpp:258-260,1069-1072`). Optional `DEATHLEAK_RENDER_ONLY_XML_IDS=6,7,10,13,14,15,16,31` isolates the curtains; repeat once without filtering to inspect real painter/alpha composition. Verify same camera and player state from logs or frame markers, nonidentical falling patterns, 7/9/11 relative motion, stable quad edges/alpha, and no visible wrap seam or flicker; compare to the prior active-play waterfall capture as context only. The Steam still is composition evidence, not a motion/rate oracle. Request human review of the two active-play frames or short GPU video after this gate.
