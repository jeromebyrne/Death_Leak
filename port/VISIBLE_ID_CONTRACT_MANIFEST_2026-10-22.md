# Visible XML ID contract manifest — forest-1 and waterfall-top (2026-10-22)

## Verdict

**No new visible high-area source-backed gap.** All 155 authored visual records are covered by the current loader path, subject to ordinary runtime asset loading and camera/update gating. Waterfall ID4 is alpha-zero and excluded from visible-gap claims. The mismatches already owned by prior audits (notably parallax follow temporal behavior and rotation) are not new leads. This is a static CPU contract audit, not a rendered parity claim.

## Reading the manifest

- Source: `Game/XmlFiles/levels/{forest_exploration_1,waterfall_top}.xml`; each row is one direct child visual object, keyed by XML ID. Nonvisual geometry, triggers, audio, and pickups outside the requested visual classes are excluded. `dims` are authored visual width×height, not collision dimensions.
- `texture` is the authored asset path relative to `Game/`. When `anim=` is present, the loader resolves the first animation frame instead of the placeholder `<texture>` (the animation XML and resolved first frame were checked on disk).
- `effect`: `L`=effectlighttexture, `F`=effectfoliagesway, `W`=effectpixelwobble. Suffix values for F/W denote authored nonzero parameters, retained in the XML; the loader reads the full foliage corner/time vector or pixel intensity/speed. `rotation` is the authored position rotation in radians.
- `parallax`: `follow x/y; auto x/y@speed; multiplier x/y`. `scroll`: StartLerpX/Y, LerpDistanceX/Y, LerpTime, FadeInTime, FadeOutTime. `particle`: count, loop/time, fade-in/out, size range/scale, speed range, lifetime range, direction, gravity and brightness are in source XML and admitted by the dedicated spray path. Dashes mean not applicable.
- `admission`: `sprite` means generic positive-dimension textured SceneSprite; `spray` means hardcoded admitted ParticleSpray; `invisible` means loaded collision/texture but alpha zero. Asset resolution was checked statically, including animation first frames; this does not guarantee runtime texture decoding.

## forest_exploration_1

| ID | kind | texture / animation | α | dims | depth | effect | rotation | follow / auto / scroll / particle | admission |
|---:|---|---|---:|---:|---|---|---:|---|---|
| 1 | ParallaxLayer | Media/objects/tree_strip_bg.png | 0.9 | 1985×1090 | kFarBackground | L | 0 | follow true/false; auto false/false@0/0; mult 5.9e-05/2.4e-05 | sprite |
| 2 | Player | Media/subzero/still1.png (anim=XmlFiles/animation/player_anim_default.xml → Media/characters/player/idle_default/idle_default_000.png) | 1 | 181.394×215.352 | kPlayer | L | 0 | — | sprite |
| 4 | Sprite | Media/levels/waterfall_top/ground.png | 1 | 3237.33×1024 | kGroundBack | L | 0 | — | sprite |
| 5 | Sprite | Media/objects/tree2.png | 1 | 1112.32×2743.38 | kGroundFront | L | 0 | — | sprite |
| 6 | Sprite | Media/objects/tree2.png | 1 | 1112.32×2743.38 | kGroundFront | L | -0.079602 | — | sprite |
| 7 | Sprite | Media/objects/tree2_blur.png | 0.7 | 1112.32×2743.38 | kFarForeground | L | 0 | — | sprite |
| 8 | Sprite | Media/objects/tree2_blur.png | 1 | 1112.32×2743.38 | kFarForeground | L | 0 | — | sprite |
| 9 | Sprite | Media/objects/tree2_blur.png | 0.7 | 1112.32×2743.38 | kFarForeground | L | 0 | — | sprite |
| 10 | Sprite | Media/objects/tree2_blur.png | 0.7 | 1686.03×4158.37 | kMiddleForeground | L | 0 | — | sprite |
| 11 | Sprite | Media/objects/raft.png | 1 | 458.299×125.316 | kGround | L | 0 | — | sprite |
| 14 | Sprite | Media/sun_shaft_2.png | 1 | 512×512 | kGroundFront | L | -1.52 | — | sprite |
| 15 | Sprite | Media/sun_shaft_2.png | 1 | 512×512 | kGroundFront | L | -1.47 | — | sprite |
| 17 | Sprite | Media/levels/waterfall_top/ground.png | 1 | 7354.12×1024 | kNearBackground | L | 0 | — | sprite |
| 18 | Sprite | Media/levels/forest_exploration_1/leaves_shadow.png | 0.4 | 10000×252 | kNearBackground | L | 0 | — | sprite |
| 19 | Sprite | Media/levels/waterfall_top/ground_blur.png | 1 | 7354.12×1024 | kMiddleBackground | L | 0 | — | sprite |
| 20 | Sprite | Media/levels/forest_exploration_1/leaves_shadow.png | 0.25 | 10000×252 | kMiddleBackground | L | 0 | — | sprite |
| 21 | Sprite | Media/objects/tree_strip_dark.png | 1 | 1985×1238 | kMiddleBackground | L | 0 | — | sprite |
| 22 | Sprite | Media/objects/tree_strip.png | 1 | 5047.11×1238 | kNearBackground | L | 0 | — | sprite |
| 23 | Sprite | Media/levels/rope.png | 1 | 256×256 | kGroundBack | L | 1.55999 | — | sprite |
| 27 | Sprite | Media/objects/branch_3.png | 1 | 395×167 | kGroundFront | L | 0 | — | sprite |
| 28 | Sprite | Media/objects/branch_5.png | 1 | 368×317 | kGroundFront | L | 0 | — | sprite |
| 29 | Sprite | Media/objects/branch_6.png | 1 | 441×174 | kGroundFront | L | 0 | — | sprite |
| 30 | Sprite | Media/objects/branch_1.png | 1 | 597.635×217.582 | kGroundFront | L | 0 | — | sprite |
| 31 | Sprite | Media/objects/branch_4.png | 1 | 555.257×100.075 | kGroundFront | L | 0.37 | — | sprite |
| 37 | ParallaxLayer | Media/levels/forest_exploration_1/shrubbery.png | 0.95 | 2048×512 | kMiddleForeground | L | 0 | follow true/false; auto false/false@0/0; mult 0.00069/0 | sprite |
| 38 | Sprite | Media/blank_white.png | 0.15 | 20000×2743.38 | kMiddleBackground | L | 0 | — | sprite |
| 39 | Sprite | Media/blank_white.png | 0.2 | 20000×2743.38 | kGroundBack | L | 0 | — | sprite |
| 40 | ParallaxLayer | Media/levels/forest_exploration_1/trees_top.png | 0.95 | 2048×677 | kMiddleForeground | L | 0 | follow true/false; auto false/false@0/0; mult 0.00069/0 | sprite |
| 41 | Sprite | Media/levels/forest_exploration_1/trees_top_small.png | 1 | 756×677 | kGroundFront | F(bottom_left_intensity_x=20,bottom_right_intensity_x=20;t=1) | 0 | — | sprite |
| 42 | Sprite | Media/levels/forest_exploration_1/trees_top_small.png | 1 | 756×677 | kGroundFront | F(bottom_left_intensity_x=20,bottom_right_intensity_x=20;t=1) | 0 | — | sprite |
| 43 | Sprite | Media/levels/forest_exploration_1/trees_top_small.png | 1 | 756×677 | kGroundFront | F(bottom_left_intensity_x=20,bottom_right_intensity_x=20;t=1) | 0 | — | sprite |
| 44 | Sprite | Media/levels/forest_exploration_1/trees_top_small.png | 1 | 756×677 | kGroundFront | F(bottom_left_intensity_x=20,bottom_right_intensity_x=20;t=1) | 0 | — | sprite |
| 45 | Sprite | Media/levels/forest_exploration_1/trees_top_small.png | 1 | 756×677 | kGroundFront | F(bottom_left_intensity_x=20,bottom_right_intensity_x=20;t=1) | 0 | — | sprite |
| 46 | Sprite | Media/levels/forest_exploration_1/leaves_shadow.png | 0.5 | 10000×252 | kGround | F(top_left_intensity_x=7,top_left_intensity_y=6,top_right_intensity_x=8,top_right_intensity_y=9;t=1) | 0 | — | sprite |
| 47 | ParticleSpray | Media/levels/forest_exploration_1/oak_leaf.png | 1 | 50000.3×2000.21 | kGroundFront | L | 0 | particle n=4; loop=true/0; fade=0.05/0.98; size=64–92/to=false:1; speed=3.5–4; life=6–40; dir=0/-1; g=0; bright=0.85–1 | spray |
| 50 | Sprite | Media/cart_tracks.png | 0.6 | 492×61 | kGround | L | 0 | — | sprite |
| 51 | Sprite | Media/cart_tracks.png | 0.6 | 492×61 | kGround | L | -8e-06 | — | sprite |
| 52 | Sprite | Media/levels/waterfall_top/ground.png | 1 | 3237.33×1024 | kGroundBack | L | 0 | — | sprite |
| 53 | Sprite | Media/levels/waterfall_bottom/cave_background_blur.png | 1 | 1091.01×818.571 | kGroundFront | L | -0.301793 | — | sprite |
| 54 | Sprite | Media/levels/waterfall_bottom/cave_background_blur.png | 1 | 1173.34×880.337 | kGroundFront | L | 0.348359 | — | sprite |
| 55 | Sprite | Media/levels/waterfall_bottom/cave_background_blur.png | 1 | 1750×1313 | kGroundFront | L | 0.348359 | — | sprite |
| 56 | Sprite | Media/edge_fade_top.png | 1 | 400×256 | kGround | L | 0 | — | sprite |
| 57 | Sprite | Media/objects/raft.png | 1 | 458.299×125.316 | kGround | L | 0.219974 | — | sprite |
| 58 | Sprite | Media/objects/raft.png | 1 | 458.299×125.316 | kGround | L | 0.219974 | — | sprite |
| 59 | Sprite | Media/levels/waterfall_bottom/cave_background_blur.png | 1 | 1145.36×859.344 | kNearBackground | L | 0.348359 | — | sprite |
| 60 | Sprite | Media/levels/waterfall_bottom/cave_background_blur.png | 1 | 1091.01×818.571 | kGroundFront | L | 0.258207 | — | sprite |
| 61 | Sprite | Media/levels/waterfall_bottom/cave_background_blur.png | 1 | 1091.01×818.571 | kGroundFront | L | 0.258207 | — | sprite |
| 66 | CurrencyOrb | Media/subzero/still1.png (anim=XmlFiles/orb_anim.xml → Media/orb/1.png) | 0.75 | 47.6973×72.8312 | kOrb | L | 0 | — | sprite |
| 68 | CurrencyOrb | Media/subzero/still1.png (anim=XmlFiles/orb_anim.xml → Media/orb/1.png) | 0.75 | 47.6973×72.8312 | kOrb | L | 0 | — | sprite |
| 70 | CurrencyOrb | Media/subzero/still1.png (anim=XmlFiles/orb_anim.xml → Media/orb/1.png) | 0.75 | 47.6973×72.8312 | kOrb | L | 0 | — | sprite |
| 72 | CurrencyOrb | Media/subzero/still1.png (anim=XmlFiles/orb_anim.xml → Media/orb/1.png) | 0.75 | 47.6973×72.8312 | kOrb | L | 0 | — | sprite |
| 80 | CurrencyOrb | Media/subzero/still1.png (anim=XmlFiles/orb_anim.xml → Media/orb/1.png) | 0.75 | 47.6973×72.8312 | kOrb | L | 0 | — | sprite |
| 84 | CurrencyOrb | Media/subzero/still1.png (anim=XmlFiles/orb_anim.xml → Media/orb/1.png) | 0.75 | 47.6973×72.8312 | kOrb | L | 0 | — | sprite |
| 85 | CurrencyOrb | Media/subzero/still1.png (anim=XmlFiles/orb_anim.xml → Media/orb/1.png) | 0.75 | 47.6973×72.8312 | kOrb | L | 0 | — | sprite |
| 86 | CurrencyOrb | Media/subzero/still1.png (anim=XmlFiles/orb_anim.xml → Media/orb/1.png) | 0.75 | 47.6973×72.8312 | kOrb | L | 0 | — | sprite |
| 88 | CurrencyOrb | Media/subzero/still1.png (anim=XmlFiles/orb_anim.xml → Media/orb/1.png) | 0.75 | 47.6973×72.8312 | kOrb | L | 0 | — | sprite |
| 90 | CurrencyOrb | Media/subzero/still1.png (anim=XmlFiles/orb_anim.xml → Media/orb/1.png) | 0.75 | 47.6973×72.8312 | kOrb | L | 0 | — | sprite |
| 92 | CurrencyOrb | Media/subzero/still1.png (anim=XmlFiles/orb_anim.xml → Media/orb/1.png) | 0.75 | 47.6973×72.8312 | kOrb | L | 0 | — | sprite |
| 94 | CurrencyOrb | Media/subzero/still1.png (anim=XmlFiles/orb_anim.xml → Media/orb/1.png) | 0.75 | 47.6973×72.8312 | kOrb | L | 0 | — | sprite |
| 96 | CurrencyOrb | Media/subzero/still1.png (anim=XmlFiles/orb_anim.xml → Media/orb/1.png) | 0.75 | 47.6973×72.8312 | kOrb | L | 0 | — | sprite |
| 100 | CurrencyOrb | Media/subzero/still1.png (anim=XmlFiles/orb_anim.xml → Media/orb/1.png) | 0.75 | 47.6973×72.8312 | kOrb | L | 0 | — | sprite |
| 102 | CurrencyOrb | Media/subzero/still1.png (anim=XmlFiles/orb_anim.xml → Media/orb/1.png) | 0.75 | 47.6973×72.8312 | kOrb | L | 0 | — | sprite |
| 104 | CurrencyOrb | Media/subzero/still1.png (anim=XmlFiles/orb_anim.xml → Media/orb/1.png) | 0.75 | 47.6973×72.8312 | kOrb | L | 0 | — | sprite |
| 105 | CurrencyOrb | Media/subzero/still1.png (anim=XmlFiles/orb_anim.xml → Media/orb/1.png) | 0.75 | 47.6973×72.8312 | kOrb | L | 0 | — | sprite |
| 107 | CurrencyOrb | Media/subzero/still1.png (anim=XmlFiles/orb_anim.xml → Media/orb/1.png) | 0.75 | 47.6973×72.8312 | kOrb | L | 0 | — | sprite |
| 112 | CurrencyOrb | Media/subzero/still1.png (anim=XmlFiles/orb_anim.xml → Media/orb/1.png) | 0.75 | 47.6973×72.8312 | kOrb | L | 0 | — | sprite |
| 113 | CurrencyOrb | Media/subzero/still1.png (anim=XmlFiles/orb_anim.xml → Media/orb/1.png) | 0.75 | 47.6973×72.8312 | kOrb | L | 0 | — | sprite |
| 114 | CurrencyOrb | Media/subzero/still1.png (anim=XmlFiles/orb_anim.xml → Media/orb/1.png) | 0.75 | 47.6973×72.8312 | kOrb | L | 0 | — | sprite |
| 115 | CurrencyOrb | Media/subzero/still1.png (anim=XmlFiles/orb_anim.xml → Media/orb/1.png) | 0.75 | 47.6973×72.8312 | kOrb | L | 0 | — | sprite |
| 119 | ScrollingSprite | Media/fog.png | 0.5 | 1988×1290 | kGroundFront | W(0.07,0.3) | 0.11 | scroll 789.985,2954,1000,0,45,6,6 | sprite |
| 122 | ScrollingSprite | Media/fog.png | 0.6 | 1988×1290 | kGroundFront | W(0.07,0.3) | 0.11 | scroll -125.015,2861,-750,0,70,6,6 | sprite |


## waterfall_top

| ID | kind | texture / animation | α | dims | depth | effect | rotation | follow / auto / scroll / particle | admission |
|---:|---|---|---:|---:|---|---|---:|---|---|
| 1 | ParallaxLayer | Media/objects/tree_strip_bg.png | 0.9 | 1985×1090 | kFarBackground | L | 0 | follow true/false; auto false/false@0/0; mult 5.9e-05/2.4e-05 | sprite |
| 2 | Player | Media/subzero/still1.png (anim=XmlFiles/animation/player_anim_default.xml → Media/characters/player/idle_default/idle_default_000.png) | 1 | 181.394×215.352 | kPlayer | L | 0 | — | sprite |
| 3 | Sprite | Media/levels/underwater_cave/cave_background.png | 1 | 2077.2×1558.49 | kNearBackground | L | 0 | — | sprite |
| 4 | SolidMovingSprite | Media/blank_black.png | 0 | 2×2 | kGround | L | 0 | — | invisible |
| 6 | ParallaxLayer | Media/waterfall_4.png | 0.5 | 841.308×3303.14 | kGroundBack | L | 0 | follow false/false; auto false/true@0/7; mult 0/0 | sprite |
| 7 | ParallaxLayer | Media/waterfall_4.png | 0.2 | 1120.14×2240.27 | kGroundFront | L | 0 | follow false/false; auto false/true@0/11; mult 0/0 | sprite |
| 8 | Sprite | Media/levels/waterfall_bottom/cave_background_blur.png | 1 | 1750×1313 | kFarForeground | L | -0.236648 | — | sprite |
| 9 | Sprite | Media/levels/waterfall_bottom/cave_background_blur.png | 1 | 1750×1313 | kFarForeground | L | 0.089148 | — | sprite |
| 10 | ParallaxLayer | Media/waterfall_4.png | 0.2 | 1114.4×2228.79 | kFarForeground | L | 0 | follow false/false; auto false/true@0/9; mult 0/0 | sprite |
| 13 | ParallaxLayer | Media/waterfall_4.png | 0.3 | 810.663×1621.33 | kGroundBack | W(0.005,10) | 0 | follow false/false; auto false/true@0/11; mult 0/0 | sprite |
| 14 | ParallaxLayer | Media/waterfall_4.png | 0.3 | 810.663×1621.33 | kGroundBack | W(0.005,10) | 0 | follow false/false; auto false/true@0/11; mult 0/0 | sprite |
| 15 | ParallaxLayer | Media/waterfall_4.png | 0.2 | 1120.14×2240.27 | kGroundFront | L | 0 | follow false/false; auto false/true@0/11; mult 0/0 | sprite |
| 16 | ParallaxLayer | Media/waterfall_4.png | 0.2 | 1114.4×2228.79 | kFarForeground | L | 0 | follow false/false; auto false/true@0/9; mult 0/0 | sprite |
| 17 | Sprite | Media/levels/waterfall_top/ground.png | 1 | 2377.01×1024 | kGroundBack | L | 0.089953 | — | sprite |
| 18 | Sprite | Media/levels/waterfall_top/ground.png | 1 | 2377.01×1024 | kGroundBack | L | 0.089953 | — | sprite |
| 19 | Sprite | Media/levels/waterfall_bottom/cave_background_blur.png | 1 | 1750×1313 | kFarForeground | L | -0.525197 | — | sprite |
| 20 | Sprite | Media/levels/waterfall_bottom/cave_background_blur.png | 1 | 1750×1313 | kFarForeground | L | -0.236648 | — | sprite |
| 22 | Sprite | Media/levels/waterfall_top/ground_blur.png | 1 | 4278.04×1024 | kMiddleBackground | L | 0 | — | sprite |
| 23 | Sprite | Media/objects/tree2.png | 0.95 | 1112.32×2743.38 | kGroundFront | L | 0 | — | sprite |
| 24 | Sprite | Media/objects/tree2.png | 1 | 1112.32×2743.38 | kGroundFront | L | -0.079602 | — | sprite |
| 25 | Sprite | Media/objects/tree_strip.png | 1 | 5047.11×1238 | kNearBackground | L | 0 | — | sprite |
| 26 | Sprite | Media/objects/tree_strip.png | 1 | 4332.9×1100 | kMiddleBackground | L | 0 | — | sprite |
| 27 | Sprite | Media/objects/tree2_blur.png | 0.95 | 1112.32×2743.38 | kFarForeground | L | 0 | — | sprite |
| 28 | Sprite | Media/objects/tree2_blur.png | 0.95 | 1112.32×2743.38 | kFarForeground | L | 0 | — | sprite |
| 29 | Sprite | Media/objects/tree2_blur.png | 0.95 | 1112.32×2743.38 | kFarForeground | L | 0 | — | sprite |
| 30 | Sprite | Media/objects/tree2_blur.png | 0.7 | 1686.03×4158.37 | kMiddleForeground | L | 0 | — | sprite |
| 31 | ParallaxLayer | Media/waterfall_4.png | 0.2 | 1269.77×4985.35 | kFarForeground | L | 0 | follow false/false; auto false/true@0/7; mult 0/0 | sprite |
| 32 | Sprite | Media/objects/raft.png | 1 | 674.042×184.308 | kGround | L | 0.18011 | — | sprite |
| 33 | Sprite | Media/objects/raft.png | 1 | 674.042×184.308 | kGround | L | -0.279275 | — | sprite |
| 36 | Sprite | Media/levels/rope_end.png | 1 | 256×256 | kGroundBack | F(bottom_left_intensity_x=10,bottom_right_intensity_x=5;t=6) | 0 | — | sprite |
| 37 | Sprite | Media/levels/rope_end.png | 1 | 256×256 | kGroundBack | F(bottom_left_intensity_x=10,bottom_right_intensity_x=5;t=6) | 0 | — | sprite |
| 38 | Sprite | Media/levels/rope_end.png | 1 | 256×256 | kGroundBack | F(bottom_left_intensity_x=10,bottom_right_intensity_x=5;t=6) | 0 | — | sprite |
| 39 | Sprite | Media/levels/rope_end.png | 1 | 256×256 | kGroundBack | F(bottom_left_intensity_x=10,bottom_right_intensity_x=5;t=6) | 0 | — | sprite |
| 41 | Sprite | Media/levels/waterfall_bottom/cave_background_blur.png | 1 | 1750×1313 | kFarForeground | L | 0 | — | sprite |
| 42 | Sprite | Media/objects/tree_strip.png | 1 | 5047.11×1238 | kNearBackground | L | 0 | — | sprite |
| 43 | Sprite | Media/objects/tree_strip.png | 1 | 4332.9×1100 | kMiddleBackground | L | 0 | — | sprite |
| 46 | Sprite | Media/sun_shaft_2.png | 1 | 512×512 | kGroundFront | L | -1.52 | — | sprite |
| 47 | Sprite | Media/sun_shaft_2.png | 1 | 512×512 | kGroundFront | L | -1.47 | — | sprite |
| 49 | ParticleSpray | Media/waterfall_splash.png | 1 | 1000×3000 | kGroundFront | L | 0 | particle n=100; loop=true/0; fade=0.25/0.25; size=250–450/to=false:4; speed=14–35; life=1–1.5; dir=0/-1; g=0; bright=1–1 | spray |
| 50 | ParticleSpray | Media/waterfall_splash.png | 1 | 300×300 | kMiddleForeground | L | 0 | particle n=10; loop=true/0; fade=0.25/0.25; size=45–90/to=true:4; speed=5–-32; life=0.5–1.5; dir=0/-1; g=22; bright=1–1 | spray |
| 51 | ParticleSpray | Media/waterfall_splash.png | 1 | 300×300 | kMiddleForeground | L | 0 | particle n=10; loop=true/0; fade=0.25/0.25; size=45–90/to=true:4; speed=5–-32; life=0.5–1.5; dir=0.01/-1; g=22; bright=1–1 | spray |
| 52 | Sprite | Media/objects/branch_3.png | 1 | 547.614×199.371 | kGroundFront | L | 0 | — | sprite |
| 53 | Sprite | Media/objects/branch_3.png | 1 | 395×167 | kGroundFront | L | 0 | — | sprite |
| 54 | Sprite | Media/objects/branch_2.png | 1 | 531×264 | kGroundFront | L | 0 | — | sprite |
| 58 | Sprite | Media/blank_white.png | 0.2 | 20000×10000 | kGroundBack | L | 0 | — | sprite |
| 59 | ParallaxLayer | Media/levels/forest_exploration_1/shrubbery.png | 0.95 | 2048×512 | kMiddleForeground | L | 0 | follow true/false; auto false/false@0/0; mult 0.00069/0 | sprite |
| 60 | Sprite | Media/levels/forest_exploration_1/trees_top_small.png | 1 | 756×677 | kGroundFront | F(bottom_left_intensity_x=20,bottom_right_intensity_x=20;t=1) | 0 | — | sprite |
| 61 | Sprite | Media/levels/forest_exploration_1/trees_top_small.png | 1 | 756×677 | kGroundFront | F(bottom_left_intensity_x=20,bottom_right_intensity_x=20;t=1) | 0 | — | sprite |
| 62 | Sprite | Media/levels/forest_exploration_1/trees_top_small.png | 1 | 592.266×530.375 | kGroundFront | F(bottom_left_intensity_x=20,bottom_right_intensity_x=20;t=1) | 0 | — | sprite |
| 63 | Sprite | Media/levels/forest_exploration_1/trees_top_small.png | 1 | 756×677 | kGroundFront | F(bottom_left_intensity_x=20,bottom_right_intensity_x=20;t=1) | 0 | — | sprite |
| 64 | Sprite | Media/levels/forest_exploration_1/trees_top.png | 1 | 2048×677 | kFarForeground | F(bottom_left_intensity_x=20,bottom_right_intensity_x=20;t=1) | 0 | — | sprite |
| 65 | Sprite | Media/levels/forest_exploration_1/trees_top.png | 1 | 2048×677 | kFarForeground | F(bottom_left_intensity_x=20,bottom_right_intensity_x=20;t=1) | 0 | — | sprite |
| 66 | Sprite | Media/levels/forest_exploration_1/leaves_shadow.png | 0.5 | 2000×252 | kGround | F(top_left_intensity_x=7,top_left_intensity_y=6,top_right_intensity_x=8,top_right_intensity_y=9;t=1) | -0.099903 | — | sprite |
| 67 | Sprite | Media/levels/forest_exploration_1/leaves_shadow.png | 0.65 | 2000×252 | kGround | F(top_left_intensity_x=7,top_left_intensity_y=6,top_right_intensity_x=8,top_right_intensity_y=9;t=1) | 0.080501 | — | sprite |
| 70 | Sprite | Media/cart_tracks.png | 0.6 | 492×61 | kGround | L | -0.060199 | — | sprite |
| 71 | Sprite | Media/cart_tracks.png | 0.6 | 492×61 | kGround | L | -0.090261 | — | sprite |
| 72 | Sprite | Media/cart_tracks.png | 0.6 | 492×61 | kGround | L | 0.099739 | — | sprite |
| 73 | Sprite | Media/cart_tracks.png | 0.6 | 492×61 | kGround | L | 0.099739 | — | sprite |
| 75 | CurrencyOrb | Media/subzero/still1.png (anim=XmlFiles/orb_anim.xml → Media/orb/1.png) | 0.75 | 47.6973×72.8312 | kOrb | L | 0 | — | sprite |
| 77 | CurrencyOrb | Media/subzero/still1.png (anim=XmlFiles/orb_anim.xml → Media/orb/1.png) | 0.75 | 47.6973×72.8312 | kOrb | L | 0 | — | sprite |
| 82 | CurrencyOrb | Media/subzero/still1.png (anim=XmlFiles/orb_anim.xml → Media/orb/1.png) | 0.75 | 47.6973×72.8312 | kOrb | L | 0 | — | sprite |
| 84 | CurrencyOrb | Media/subzero/still1.png (anim=XmlFiles/orb_anim.xml → Media/orb/1.png) | 0.75 | 47.6973×72.8312 | kOrb | L | 0 | — | sprite |
| 86 | CurrencyOrb | Media/subzero/still1.png (anim=XmlFiles/orb_anim.xml → Media/orb/1.png) | 0.75 | 47.6973×72.8312 | kOrb | L | 0 | — | sprite |
| 88 | CurrencyOrb | Media/subzero/still1.png (anim=XmlFiles/orb_anim.xml → Media/orb/1.png) | 0.75 | 47.6973×72.8312 | kOrb | L | 0 | — | sprite |
| 89 | CurrencyOrb | Media/subzero/still1.png (anim=XmlFiles/orb_anim.xml → Media/orb/1.png) | 0.75 | 47.6973×72.8312 | kOrb | L | 0 | — | sprite |
| 91 | CurrencyOrb | Media/subzero/still1.png (anim=XmlFiles/orb_anim.xml → Media/orb/1.png) | 0.75 | 47.6973×72.8312 | kOrb | L | 0 | — | sprite |
| 98 | CurrencyOrb | Media/subzero/still1.png (anim=XmlFiles/orb_anim.xml → Media/orb/1.png) | 0.75 | 47.6973×72.8312 | kOrb | L | 0 | — | sprite |
| 100 | CurrencyOrb | Media/subzero/still1.png (anim=XmlFiles/orb_anim.xml → Media/orb/1.png) | 0.75 | 47.6973×72.8312 | kOrb | L | 0 | — | sprite |
| 101 | CurrencyOrb | Media/subzero/still1.png (anim=XmlFiles/orb_anim.xml → Media/orb/1.png) | 0.75 | 47.6973×72.8312 | kOrb | L | 0 | — | sprite |
| 102 | CurrencyOrb | Media/subzero/still1.png (anim=XmlFiles/orb_anim.xml → Media/orb/1.png) | 0.75 | 47.6973×72.8312 | kOrb | L | 0 | — | sprite |
| 103 | CurrencyOrb | Media/subzero/still1.png (anim=XmlFiles/orb_anim.xml → Media/orb/1.png) | 0.75 | 47.6973×72.8312 | kOrb | L | 0 | — | sprite |
| 104 | CurrencyOrb | Media/subzero/still1.png (anim=XmlFiles/orb_anim.xml → Media/orb/1.png) | 0.75 | 47.6973×72.8312 | kOrb | L | 0 | — | sprite |
| 105 | CurrencyOrb | Media/subzero/still1.png (anim=XmlFiles/orb_anim.xml → Media/orb/1.png) | 0.75 | 47.6973×72.8312 | kOrb | L | 0 | — | sprite |
| 108 | CurrencyOrb | Media/subzero/still1.png (anim=XmlFiles/orb_anim.xml → Media/orb/1.png) | 0.75 | 47.6973×72.8312 | kOrb | L | 0 | — | sprite |
| 109 | CurrencyOrb | Media/subzero/still1.png (anim=XmlFiles/orb_anim.xml → Media/orb/1.png) | 0.75 | 47.6973×72.8312 | kOrb | L | 0 | — | sprite |
| 110 | CurrencyOrb | Media/subzero/still1.png (anim=XmlFiles/orb_anim.xml → Media/orb/1.png) | 0.75 | 47.6973×72.8312 | kOrb | L | 0 | — | sprite |
| 111 | CurrencyOrb | Media/subzero/still1.png (anim=XmlFiles/orb_anim.xml → Media/orb/1.png) | 0.75 | 47.6973×72.8312 | kOrb | L | 0 | — | sprite |
| 112 | CurrencyOrb | Media/subzero/still1.png (anim=XmlFiles/orb_anim.xml → Media/orb/1.png) | 0.75 | 47.6973×72.8312 | kOrb | L | 0 | — | sprite |
| 113 | CurrencyOrb | Media/subzero/still1.png (anim=XmlFiles/orb_anim.xml → Media/orb/1.png) | 0.75 | 47.6973×72.8312 | kOrb | L | 0 | — | sprite |
| 115 | CurrencyOrb | Media/subzero/still1.png (anim=XmlFiles/orb_anim.xml → Media/orb/1.png) | 0.75 | 47.6973×72.8312 | kOrb | L | 0 | — | sprite |
| 117 | ScrollingSprite | Media/fog.png | 0.6 | 1988×1290 | kGroundFront | W(0.1,0.1) | 0.11 | scroll -1675.01,2873,1250,0,70,2,6 | sprite |
| 118 | ScrollingSprite | Media/fog.png | 0.6 | 1988×1290 | kGroundFront | W(0.1,0.1) | 0.11 | scroll -1103.01,2712,400,0,110,6,6 | sprite |
| 119 | ScrollingSprite | Media/fog.png | 0.6 | 1988×1290 | kGroundFront | W(0.1,0.1) | 0.11 | scroll 1508.99,2879,1250,0,70,6,6 | sprite |
| 121 | ScrollingSprite | Media/fog.png | 0.5 | 1988×1290 | kGroundFront | W(0.1,0.1) | 0.11 | scroll -1179.01,2893,-600,0,59,6,6 | sprite |


## Port comparison and exclusions

- `port/game/LevelScene.cpp:271–324` explicitly admits the only four spray IDs, and reads their texture, position/dimensions/depth, loop/fade/size/speed/lifetime/direction/gravity/brightness. The other requested visual classes enter the generic textured loader at `:328–360`; their IDs are stored at `:362–366`, with positive dimensions required at `:536`.
- Parallax X/Y follow, multipliers and Y autoscroll are read at `:367–376`; all authored `autoscrollx` flags in these two levels are false, so absent X autoscroll handling has no active ID here. ScrollingSprite lerp/fade fields are read at `:519–529`.
- Animated Player/CurrencyOrb first frame (and their dedicated animation sequences), position/dimensions/rotation/depth, effect sway/wobble, flips, repeat-X, alpha and occlusion fade are handled at `:378–535`. Source `effectlighttexture` is the normal textured baseline. Port rendering paths remain subject to separate prior temporal/rotation/particle audits.
- Waterfall `SolidMovingSprite` ID4 is `Media/blank_black.png`, α=0, 2×2 **visual** dimensions; its large collision dimensions are separate and loaded at `:261–270`. It is not a visible high-area missing layer.
- The authored Player/CurrencyOrb `Media/subzero/still1.png` placeholder is absent on disk, but each is `isanimated=true`; `AnimationFrame` overrides it with an existing first frame before texture admission (`:328–345`). This is not a missing visual asset.

## Static check result

- Counts: {'ParallaxLayer': 13, 'Player': 2, 'Sprite': 85, 'ParticleSpray': 4, 'CurrencyOrb': 44, 'ScrollingSprite': 6, 'SolidMovingSprite': 1}.
- Missing resolved assets, nonpositive dimensions, unknown effects, unsupported active autoscroll X, or non-whitelisted spray IDs: none.
- No build, game launch, display/GPU, source/test edit, shared checkpoint edit, commit or push was performed. Only this report was written.
