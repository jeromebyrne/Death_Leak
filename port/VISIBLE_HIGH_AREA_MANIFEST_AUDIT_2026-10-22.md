# Visible high-area XML manifest audit — 2026-10-22

**Verdict: no new source-backed discrepancy in the admitted fields.** This is a source/CPU contract comparison, not a rendered parity claim.

Scope: direct `Sprite`, `ParallaxLayer`, and `ScrollingSprite` children of the two level XMLs with authored alpha > 0 and visual width×height ≥ 500,000 world-square units. This explicit area cutoff includes the broad ground, tree, waterfall, broad atmosphere and fog layers while excluding small props, orbs, player, collision-only `SolidLineStrip`, audio/triggers, and all particle sprays. Alpha-zero waterfall ID4 is excluded. Rows with identical admitted fields are grouped by ID; positions and rotations are deliberately outside this lane.

Columns: `F` = authored horizontal/vertical flip (`0` false, `1` true); `P` = parallax follow X/Y, camera multiplier X/Y, auto-scroll X/Y and speed X/Y; `S` = scrolling start X/Y, distance X/Y, time, fade-in/out. Dash means the field is inapplicable to the object class. Texture paths are relative to `Game/`. `L/F/W` effects mean `effectlighttexture`/`effectfoliagesway`/`effectpixelwobble`; the XML effect and its properties remain the evidence of the exact shader parameters.

## forest_exploration_1 (33 objects; 25 contract rows)

| ID(s) | Class | Texture | W×H | α | Effect | Depth | P / S | F XY |
|---|---|---|---:|---:|---|---|---|---|
| 1 | ParallaxLayer | Media/objects/tree_strip_bg.png | 1985×1090 | 0.9 | L | kFarBackground | P 1/0; 0.000059/0.000024; 0/0@0.000000/0.000000 | 00 |
| 4,52 | Sprite | Media/levels/waterfall_top/ground.png | 3237.332×1024 | 1 | L | kGroundBack | — | 10 |
| 5,6 | Sprite | Media/objects/tree2.png | 1112.317×2743.384 | 1 | L | kGroundFront | — | 10 |
| 7,9 | Sprite | Media/objects/tree2_blur.png | 1112.317×2743.384 | 0.7 | L | kFarForeground | — | 10 |
| 8 | Sprite | Media/objects/tree2_blur.png | 1112.317×2743.384 | 1 | L | kFarForeground | — | 10 |
| 10 | Sprite | Media/objects/tree2_blur.png | 1686.031×4158.374 | 0.7 | L | kMiddleForeground | — | 10 |
| 17 | Sprite | Media/levels/waterfall_top/ground.png | 7354.12×1024 | 1 | L | kNearBackground | — | 10 |
| 18 | Sprite | Media/levels/forest_exploration_1/leaves_shadow.png | 10000×252 | 0.4 | L | kNearBackground | — | 10 |
| 19 | Sprite | Media/levels/waterfall_top/ground_blur.png | 7354.12×1024 | 1 | L | kMiddleBackground | — | 10 |
| 20 | Sprite | Media/levels/forest_exploration_1/leaves_shadow.png | 10000×252 | 0.25 | L | kMiddleBackground | — | 00 |
| 21 | Sprite | Media/objects/tree_strip_dark.png | 1985×1238 | 1 | L | kMiddleBackground | — | 00 |
| 22 | Sprite | Media/objects/tree_strip.png | 5047.11×1238 | 1 | L | kNearBackground | — | 10 |
| 37 | ParallaxLayer | Media/levels/forest_exploration_1/shrubbery.png | 2048×512 | 0.95 | L | kMiddleForeground | P 1/0; 0.000690/0.000000; 0/0@0.000000/0.000000 | 00 |
| 38 | Sprite | Media/blank_white.png | 20000×2743.384 | 0.15 | L | kMiddleBackground | — | 10 |
| 39 | Sprite | Media/blank_white.png | 20000×2743.384 | 0.2 | L | kGroundBack | — | 10 |
| 40 | ParallaxLayer | Media/levels/forest_exploration_1/trees_top.png | 2048×677 | 0.95 | L | kMiddleForeground | P 1/0; 0.000690/0.000000; 0/0@0.000000/0.000000 | 00 |
| 41 | Sprite | Media/levels/forest_exploration_1/trees_top_small.png | 756×677 | 1 | F | kGroundFront | — | 10 |
| 42,43,44,45 | Sprite | Media/levels/forest_exploration_1/trees_top_small.png | 756×677 | 1 | F | kGroundFront | — | 00 |
| 46 | Sprite | Media/levels/forest_exploration_1/leaves_shadow.png | 10000×252 | 0.5 | F | kGround | — | 10 |
| 53,60,61 | Sprite | Media/levels/waterfall_bottom/cave_background_blur.png | 1091.013×818.571 | 1 | L | kGroundFront | — | 00 |
| 54 | Sprite | Media/levels/waterfall_bottom/cave_background_blur.png | 1173.336×880.337 | 1 | L | kGroundFront | — | 00 |
| 55 | Sprite | Media/levels/waterfall_bottom/cave_background_blur.png | 1750×1313 | 1 | L | kGroundFront | — | 00 |
| 59 | Sprite | Media/levels/waterfall_bottom/cave_background_blur.png | 1145.355×859.344 | 1 | L | kNearBackground | — | 00 |
| 119 | ScrollingSprite | Media/fog.png | 1988×1290 | 0.5 | W | kGroundFront | S 789.985352,2954,1000,0,45,6,6 | 00 |
| 122 | ScrollingSprite | Media/fog.png | 1988×1290 | 0.6 | W | kGroundFront | S -125.014648,2861,-750,0,70,6,6 | 01 |

## waterfall_top (41 objects; 29 contract rows)

| ID(s) | Class | Texture | W×H | α | Effect | Depth | P / S | F XY |
|---|---|---|---:|---:|---|---|---|---|
| 1 | ParallaxLayer | Media/objects/tree_strip_bg.png | 1985×1090 | 0.9 | L | kFarBackground | P 1/0; 0.000059/0.000024; 0/0@0.000000/0.000000 | 00 |
| 3 | Sprite | Media/levels/underwater_cave/cave_background.png | 2077.2×1558.494 | 1 | L | kNearBackground | — | 00 |
| 6 | ParallaxLayer | Media/waterfall_4.png | 841.308×3303.137 | 0.5 | L | kGroundBack | P 0/0; 0.000000/0.000000; 0/1@0.000000/7.000000 | 10 |
| 7,15 | ParallaxLayer | Media/waterfall_4.png | 1120.137×2240.274 | 0.2 | L | kGroundFront | P 0/0; 0.000000/0.000000; 0/1@0.000000/11.000000 | 10 |
| 8,9,19 | Sprite | Media/levels/waterfall_bottom/cave_background_blur.png | 1750×1313 | 1 | L | kFarForeground | — | 10 |
| 10,16 | ParallaxLayer | Media/waterfall_4.png | 1114.396×2228.792 | 0.2 | L | kFarForeground | P 0/0; 0.000000/0.000000; 0/1@0.000000/9.000000 | 10 |
| 13,14 | ParallaxLayer | Media/waterfall_4.png | 810.663×1621.326 | 0.3 | W | kGroundBack | P 0/0; 0.000000/0.000000; 0/1@0.000000/11.000000 | 10 |
| 17 | Sprite | Media/levels/waterfall_top/ground.png | 2377.011×1024 | 1 | L | kGroundBack | — | 10 |
| 18 | Sprite | Media/levels/waterfall_top/ground.png | 2377.011×1024 | 1 | L | kGroundBack | — | 00 |
| 20,41 | Sprite | Media/levels/waterfall_bottom/cave_background_blur.png | 1750×1313 | 1 | L | kFarForeground | — | 11 |
| 22 | Sprite | Media/levels/waterfall_top/ground_blur.png | 4278.037×1024 | 1 | L | kMiddleBackground | — | 10 |
| 23 | Sprite | Media/objects/tree2.png | 1112.317×2743.384 | 0.95 | L | kGroundFront | — | 10 |
| 24 | Sprite | Media/objects/tree2.png | 1112.317×2743.384 | 1 | L | kGroundFront | — | 10 |
| 25,42 | Sprite | Media/objects/tree_strip.png | 5047.11×1238 | 1 | L | kNearBackground | — | 10 |
| 26,43 | Sprite | Media/objects/tree_strip.png | 4332.903×1100 | 1 | L | kMiddleBackground | — | 00 |
| 27,28,29 | Sprite | Media/objects/tree2_blur.png | 1112.317×2743.384 | 0.95 | L | kFarForeground | — | 10 |
| 30 | Sprite | Media/objects/tree2_blur.png | 1686.031×4158.374 | 0.7 | L | kMiddleForeground | — | 10 |
| 31 | ParallaxLayer | Media/waterfall_4.png | 1269.767×4985.354 | 0.2 | L | kFarForeground | P 0/0; 0.000000/0.000000; 0/1@0.000000/7.000000 | 10 |
| 58 | Sprite | Media/blank_white.png | 20000×10000 | 0.2 | L | kGroundBack | — | 10 |
| 59 | ParallaxLayer | Media/levels/forest_exploration_1/shrubbery.png | 2048×512 | 0.95 | L | kMiddleForeground | P 1/0; 0.000690/0.000000; 0/0@0.000000/0.000000 | 00 |
| 60 | Sprite | Media/levels/forest_exploration_1/trees_top_small.png | 756×677 | 1 | F | kGroundFront | — | 10 |
| 61,63 | Sprite | Media/levels/forest_exploration_1/trees_top_small.png | 756×677 | 1 | F | kGroundFront | — | 00 |
| 64,65 | Sprite | Media/levels/forest_exploration_1/trees_top.png | 2048×677 | 1 | F | kFarForeground | — | 00 |
| 66 | Sprite | Media/levels/forest_exploration_1/leaves_shadow.png | 2000×252 | 0.5 | F | kGround | — | 10 |
| 67 | Sprite | Media/levels/forest_exploration_1/leaves_shadow.png | 2000×252 | 0.65 | F | kGround | — | 10 |
| 117 | ScrollingSprite | Media/fog.png | 1988×1290 | 0.6 | W | kGroundFront | S -1675.014648,2873,1250,0,70,2,6 | 00 |
| 118 | ScrollingSprite | Media/fog.png | 1988×1290 | 0.6 | W | kGroundFront | S -1103.014648,2712,400,0,110,6,6 | 11 |
| 119 | ScrollingSprite | Media/fog.png | 1988×1290 | 0.6 | W | kGroundFront | S 1508.985352,2879,1250,0,70,6,6 | 10 |
| 121 | ScrollingSprite | Media/fog.png | 1988×1290 | 0.5 | W | kGroundFront | S -1179.014648,2893,-600,0,59,6,6 | 11 |

## Loader/draw comparison and disposition

- `LevelScene::Load` admits each nonzero, positive-size row through texture normalization/existence and the generic sprite path (`port/game/LevelScene.cpp:390-415, 549-571`). It copies authored dimensions, depth, effect parameters, alpha, and both flips (`:475-543, 549-560`). `ParallaxLayer` follow X/Y, camera multipliers, and authored Y auto-scroll/speed are read at `:414-424`; `ScrollingSprite` start/distance/time/fades are read at `:551-560`. All high-area parallax rows have `autoscrollx=false`, so the absent X-auto-scroll route is not a discrepancy for these IDs.
- `Draw` uses the sorted descending-depth sprite stream and authored texture/size (`port/game/LevelScene.cpp:1164-1226`), camera-follow and parallax UV/Y phase (`:1200-1239`), alpha and effect/flip routing (`:1240-1267`), then emits the sprite (`:1275-1280`). Scroll fade/position is advanced with update-zone gating (`:878-922`). The port’s SDL vertical-flip inversion for ordinary copies is an explicit coordinate-convention conversion, not evidence of an authored flip omission (`:1251-1267`).
- **No corrective source edit nominated.** Every admitted manifest field has a corresponding current loader/draw route; this audit does not establish equal pixels, blend/sampling behavior, camera timing, or transient gameplay parity. The next bounded corrective task is conditional: if a matched-state reference pair identifies one of these IDs as residual, isolate that ID and one field with a focused CPU draw-state assertion before implementing anything. Until then, close this source/CPU lane.

Evidence boundary: XML and C++ read-only inspection only. No build, GPU/Xorg, Docker, source/checkpoint edit, or modification outside this report.
