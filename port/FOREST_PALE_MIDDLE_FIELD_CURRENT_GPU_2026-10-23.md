# Forest-1 pale/right middle field — current frozen-frame GPU attribution (2026-10-23)

## Verdict

The post-ID37 active-play clip's broad pale/right middle-background appearance is a **composite of distinct authored layers**, not one demonstrated Linux renderer error. At the frozen current-tree `B:300,F:240` Forest-1 frame, hiding **ID15** removes the localized right-hand straight-looking light slab; **IDs38/39** brighten broad overlapping areas without defining that slab's local silhouette; **IDs19/17** contribute separate lower, approximately horizontal ground transitions. The source XML, PNG alpha, original shader/blend path, and current port path all support those roles. No tightly source-backed renderer correction is established, so no source or art was edited. This is current-Linux causal attribution, **not** matched-Windows visual parity.

## Registered capture

- HEAD `c1756b2f5b8b2c95b0566ffa7126b2b70ab4891b`, with unrelated dirty state preserved in `port/build/forest-pale-middle-current-gpu-2026-10-23/tree-status.txt`. Copied the integrated post-ID37 binary unchanged to that ignored evidence directory; both original and frozen executable SHA-256 are `b60b1e42a91c232d24470242d2eacd8edecc6e8d61c2f6b49cf949ef7ed2bf4b`. It is the integrated current dirty-tree build, **not** a pristine HEAD build. Shared build artifacts were not overwritten.
- Sequential modes: `baseline`, `hide-15`, `only-15`, `hide-38`, `hide-39`, `hide-17`, `hide-19`. Each ran the same frozen executable with `--play Game/XmlFiles/levels/forest_exploration_1.xml --script B:300,F:240 --frames 540 --fixed-dt 0.0166667 --capture <mode>.bmp`; diagnostic render-only `DEATHLEAK_RENDER_HIDE_XML_IDS`/`DEATHLEAK_RENDER_ONLY_XML_IDS` changed only per named mode. One exclusive Docker container used physical Xorg `:0`, LightDM Xauthority, `/dev/dri`, `SDL_VIDEODRIVER=x11`, and `SDL_RENDER_DRIVER=opengl`; no offscreen/software substitution. All seven logs report `accelerated=1 software=0 mode=strict-accelerated`, `nouveau/NVE7`, GPU Sepia initialization, 71 original sprites and zero missing assets. Every endpoint reports player `(2360.52,3114.74)`, five orbs and the same printed gameplay state; `elapsed` is wall time and varies, so whole log lines are not byte-identical.
- Seven lossless 1280×720 BMPs, logs, and inspection contact sheet are in the ignored evidence directory. Baseline SHA-256 is `3a4386dbc6ea2589759c0db9bfdfa08d16289625c9d5762c0d68b678b3b4009a`. This deterministic route resembles the 9-second region of the post-ID37 10-second clip, but video acquisition is not frame-locked to this state; no video-frame pixel subtraction is claimed.

## Per-ID lossless deltas

Compared with baseline, a changed pixel means any nonzero decoded RGB-channel difference. Bounding boxes are half-open top-down image coordinates. ROI counts are changed pixels, not summed brightness or exclusive ownership; overlap/occlusion matters.

| Hidden ID | Full-frame changed pixels / bbox | Right ROI x900–1279, y250–619 | Middle ROI x250–999, y380–559 | Attribution |
| --- | ---: | ---: | ---: | --- |
| 15 | 78,204 / `(1012,316,1280,649)` | 73,394 | **0** | Local right pale slab; isolated `only-15` draws its authored broad shaft shape. |
| 38 | 268,198 / `(0,0,1184,542)` | 43,989 | 43,962 | Broad white brightening behind/through other layers. |
| 39 | 541,118 / `(0,0,1280,632)` | 128,066 | 107,792 | Even broader white brightening, including middle field. |
| 19 | 27,600 / `(0,443,1178,557)` | 9,586 | 19,368 | Upper ground-strip transition where exposed. |
| 17 | 27,983 / `(0,509,1228,634)` | 7,451 | 9,511 | Lower ground-strip transition where exposed. |

For concrete samples: `(1200,400)` baseline `(116,100,72)` becomes `(61,53,37)` hiding 15, `(116,100,72)` hiding 38, and `(97,84,61)` hiding 39. At `(640,520)`, hiding 15 or 38 changes nothing, hiding 39 changes `(44,38,27)` to `(34,29,21)`, and hiding 17 changes it to `(53,46,33)`. The `only-15` image is an isolation visualization, **not** a numerical full-scene delta: its non-XML background/HUD differ from baseline. The prior same-route, pre-rotation GPU gate (`PALE_PATCH_GPU_AUDIT.md`) independently found the ID15 hide/only shaft identity; the present captures recheck it after integrated rotation/ID37 work. The separate current `R:180` ID17/19/22 gates explain why strip contributions vary with column and later tree-strip occlusion; these frame routes must not be cross-subtracted.

## Original source and port contract

- Forest XML `Game/XmlFiles/levels/forest_exploration_1.xml:237–251` authors ID15 as `sun_shaft_2.png`, 512×512, `kGroundFront`, rotation `-1.469999`, alpha 1, ordinary `effectlighttexture`. The source RGBA asset is a yellow, translucent 512×512 field with near-rectangular body and feathered/transparent margins, so a fairly straight-looking local edge is authored. The current hide bbox lies on the right and does not touch the middle ROI.
- XML `:636–665` authors IDs38/39 as 20,000×2743 `blank_white.png` quads at alpha .15/.20 and different depths. That immutable 2×2 RGBA texture is fully opaque white; the authored object alpha supplies broad brightening. Neither has the localized right-edge geometry of ID15. XML `:277–318` authors IDs17/19 as X-repeated, X-flipped `ground.png`/`ground_blur.png` ordinary light-texture strips, both alpha 1. Their measured y443/y509 hide onsets correspond to different lower texture/overdraw bands, not the upper/right ID15 slab.
- Original `Game/src/GameObjects/Sprite.cpp:194–235,626–672` draws the authored quad/UV and sends sprite alpha to `BasicLightTexture.fx:11–16,53–59`, which samples RGBA with wrapped UV and multiplies alpha. `Game/src/Core/Graphics.cpp:392–407` uses source-alpha-over blending. The original shader has no procedural softening or deletion of the shaft/white overlays. Current `port/game/LevelScene.cpp:1227–1306` sorts authored depths and submits tint, opacity, flips and rotation; `port/linux/SDLRenderer.cpp:204–267` performs the ordinary texture draw. Source state and observed hide masks agree at the level needed to identify these layers. They do **not** prove byte-identical Windows/GL sampling, mip choice, or postprocess color.

## Exact next gate and ownership

Do **not** remove/recolor ID15 or attenuate IDs38/39/17/19 on this evidence. To decide whether the appearance is a *port discrepancy*, capture a lossless original-Windows Forest-1 frame at the **same camera XY, viewport, gameplay simulation state, shader/effect phase, draw order, and sampler/resource state**, with ID15 and IDs38/39 isolated or individually disabled (or a graphics trace of their draws). Compare registered right-shaft contour/color and broad-overlay response to this frozen Linux packet, then only patch a specific demonstrated divergent renderer contract. The current Steam JPEG and active-play MP4 are not registered enough for that decision. No source-backed correction or commit/push resulted.

Only this report was written outside ignored evidence; unrelated dirty edits, `PORT_WORK_STATUS.md`, and `CHECKPOINT_STATUS.md` were untouched. The capture container exited, no game/capture process remains, and the **exclusive physical Xorg :0 GPU/build/capture lane is released**.
