# Forest middle-band UV attribution — 2026-10-22

## Verdict and scoped correction

A source-contract mismatch was confirmed for **ordinary** Linux sprite texture copies, including forest-1 ground IDs 17/19. Windows `Sprite::RecalculateVertices` assigns texture V=0 to the world-bottom vertices and V=1 to the world-top vertices (`Game/src/GameObjects/Sprite.cpp:19-22,194-235`); `Camera2D` uses positive world Y upward and an orthographic projection (`Game/src/Core/Camera2D.cpp:27-30`). The Linux projection uses screen Y downward. Before this change, `LevelScene::Draw` inverted SDL's top-down texture mapping only for `ParallaxLayer`, leaving ordinary `Sprite` IDs 17/19 vertically reversed relative to the source draw contract. Both have XML `verticalflip=false`; their authored `ground.png`/`ground_blur.png` green-to-transparent vertical profiles therefore formed hard edges too high in the Linux frame.

The fix makes ordinary SDL texture copies use `flipY = !authoredVerticalFlip`, preserving existing parallax and player behavior. The foliage-sway and pixel-wobble custom drawing branches retain their pre-existing authored-flag input because they calculate their own source-space vertices/UVs; those effects need a separate shader-specific orientation review. No XML, art, ID37, sepia, depth ordering, or gameplay behavior was edited. ID38/39 are 2×2 solid-white overlays (`blank_white.png`) at authored alphas 0.15/0.20; Y inversion is visually immaterial for them.

## Deterministic physical-GPU attribution

Evidence lives in ignored `port/build/middle-band-attribution-2026-10-22/`. A **temporary** last-frame pre-sepia `SaveFrame` hook captured the already-composited scene immediately before `ApplyScenePostprocess`; it was then removed from final source. The ordinary `--capture` produced the matching post-sepia+HUD frame. Each variant ran as a fresh process, sequentially and alone, on physical Xorg `:0`/OpenGL nouveau NVE7 in `deathleak-video-capture:bookworm`, 1280×720, forest-1, `--script R:180 --frames 180 --fixed-dt 0.0166667`. Logs confirm strict accelerated renderer, GPU `Sepia.fx`, 71 loaded sprites/zero missing assets, and identical endpoint player `(1368.99,3107.3)`/five orbs. Pre/post are the **same render frame**, not different simulation frames.

The old baseline and per-ID hide/only captures establish visible contributions. Pixel-difference boxes below use RGB difference after rendering; counts include all changed pixels, not only hard-edge pixels.

| Hide ID | Pre-sepia changed box / pixels | Post-sepia changed box / pixels | Interpretation |
| --- | --- | --- | --- |
| 17 | `[0,467,1280,604)` / 46,230 | `[0,467,1280,606)` / 46,149 | Authored near-background ground lower-middle strip. |
| 19 | `[0,405,1280,467)` / 38,210 | `[0,405,1280,467)` / 37,999 | Authored middle-background blurred ground strip. |
| 38 | `[0,0,1280,467)` / 305,635 | `[0,0,1280,467)` / 302,012 | White atmospheric overlay brightens the upper/middle scene; no texture edge. |
| 39 | `[0,0,1280,693)` / 619,779 | `[0,0,1280,691)` / 616,019 | White atmospheric overlay brightens most of the scene; no texture edge. |

At x=640 in **only-ID, pre-sepia** images, the green-dominant ground begins at y467 for ID17 and y405 for ID19 before the fix. With the source-conformant V mapping it begins at y516 and y467 respectively. This controlled asset-level movement demonstrates that the old high horizontal edges were a Linux UV defect. The old baseline and final baseline differ in 624,749 post-sepia RGB pixels; this is **not** a Windows comparison or a claim that all changed pixels are improvements. At center y440 the old/final post-sepia baseline is `(66,58,41)`/`(77,66,47)`; at y500 it is `(63,55,39)`/`(73,63,45)`. Other authored layers and shader effects continue to overlap the ground.

The final binary (without the temporary capture hook) was freshly rebuilt and captured as `final-baseline-post.bmp`, `final-only-17-post.bmp`, and `final-only-19-post.bmp` with matching logs. Its isolated ID17/19 post-sepia captures are byte-identical to those from the temporary-hook build after the UV correction. The final baseline keeps the custom effect input untouched; it is the representative full-scene result.

## Verification and limits

`verify.log` records five focused XML-to-render UV checks (IDs 17, 19, 37, 119, 122), the existing ScrollingSprite 119/122 behavior test, and a successful Docker `make -C port -j2`; only existing TinyXML fallthrough warnings occurred. `git diff --check` passed. The new focused test checks ordinary, parallax, and custom-effect branches rather than comparing against a saved self-generated frame.

No matched Windows forest-1 frame with the same camera/state/viewport/effect clock exists in the available evidence; these results prove a source-code draw-contract correction and controlled Linux pixels, **not** exact Windows visual parity or user acceptance. The next priority is a fresh active-gameplay review of the final scene, then a shader-specific orientation/UV audit for foliage sway and pixel wobble if the reviewed footage still shows material middle-band artifacts. Keep ID37's authored near-black lower foreground unchanged absent a matched original frame.
