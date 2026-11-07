# Rotated repeat-X CPU oracle — source audit (2026-10-23 23:57Z)

Scope: read-only audit of `Game/src/GameObjects/Sprite.cpp`, `Game/ShaderFiles/BasicLightTexture.fx`, `port/linux/SDLRenderer.cpp`, and their submission/transform helpers. This file is the only write. No source/test/build/status edit, Docker, or GPU run. The separate 23:55Z visibility gate establishes frame-424 admission; this report does not re-audit that claim. It is a source-contract/oracle recommendation, not a measured Windows pixel match.

## Exact source contract for Waterfall ID17

`Game/XmlFiles/levels/waterfall_top.xml:357-370` gives ID17 dimensions 2377.010986 × 1024, `native=false`, `repeatX=true`, X flip true, Y flip false, rotation +0.089953 radians, ordinary `effectlighttexture`, and `ground.png`. Texture width is 734 (as established by prior PNG inspection). In `Sprite::Initialise` (`Sprite.cpp:75-103`), authored flips negate matrix scales. `Sprite::RecalculateVertices` (`:194-234`) creates local BL/BR/TR/TL with U 0/1/1/0, then replaces both right U values by `m_dimensions.X / mTextureDimensions.X` when repeatX and non-native. Therefore **R = 2377.010986/734 = 3.2384345858** full source spans, including a fractional 0.2384345858 span. Flip/rotation do not rewrite UVs; the world transform carries the UV-labeled vertices. `GameObject.cpp:203-211` constructs translation/scale/rotation matrices; do not infer an SDL UV reset from that transform. `BasicLightTexture.fx:10-17,33-59` passes UV unchanged, samples with U/V `WRAP`, MIN_LINEAR/MAG_POINT/MIP_LINEAR, multiplies texture by lightColor and then alpha. For this ordinary untinted ID17, source alpha is preserved. D3D wraps/interpolates on **one quad**, including its fractional terminal span.

Port `LevelScene.cpp:1388-1448` creates destination, `draw.repeatX`, flip flags and corrected `OrdinarySpriteRotationDegrees` (helper in `port/render/OrdinarySpriteRotation.h`), then submits at the scene painter slot. For ID17 the resulting SDL angle is +5.153927 degrees because authored X-only reflection changes the sign relation; use that already-corrected angle, **not** another sign correction. The packet's projected destination is `(-159.0134,316.0277,1584.6741,682.6667)` at frame 424. Camera/zoom change geometry but **R remains authored width / source width**; dividing projected width by an unzoomed tile width would alter phase under zoom. The port ordinary `draw.flipY` is `!object.flipY` (`LevelScene.cpp:1422-1428`), hence true for ID17. For a *private PNG uploaded bottom-first* (`SDLRenderer.cpp:870`), the established `DrawRaftMipExperiment` convention (`:335-336`) maps screen-top V=0 when `sprite.flipY=true`, bottom V=1. This is backend orientation bookkeeping, not an extra authored vertical reflection.

`SDLRenderer.cpp:246-280` currently enters its ordinary repeat-X tile branch only at **zero rotation**; the rotated ID17 reaches a single full-source `SDL_RenderCopyExF`. Its width is stretched across the whole destination, so it cannot reproduce R=3.2384. The `RepeatX.h` culler/tiler uses axis-aligned screen X and is unsuitable for rotated corners. `SDLRenderer.cpp:668-680` also restricts foliage repeat tiling to zero rotation; that is a different effect and must not define ordinary ID17 behavior.

## Independent mathematical CPU oracle

Use a tiny asymmetric source array, e.g. 7×5 RGBA with distinct columns, top/bottom rows, transparent marker(s), and a seam marker. No renderer code in the reference. Let `W,H` be projected quad size, `C` its center, `theta` the already-corrected SDL angle, `T` source width, and `R = authoredWidth/T`. For each output pixel center `P=(x+0.5,y+0.5)`:

```text
 d = P - C
 localX =  cos(theta)*d.x + sin(theta)*d.y
 localY = -sin(theta)*d.x + cos(theta)*d.y
 if abs(localX) >= W/2 or abs(localY) >= H/2: leave destination unchanged
 q = localX/W + 0.5                  # local quad coordinate, 0 left, 1 right
 t = localY/H + 0.5                  # screen top -> bottom
 u = R*(1-q) if flipX else R*q       # do not restart at viewport/tile boundary
 v = (1-t) if privatePngTopVIs1 else t
 if flipY: v = 1-v                  # flipY is port SDL-space flag
 i = floor(fract(u)*T) % T           # nearest landmark reference; wrap 1 -> 0
 j = floor(fract(v)*sourceHeight) % sourceHeight
 src = image[j][i]
 out = src over previousPainterPixel  # source-over; respect src.a * tint.a * opacity
```

For the actual bottom-first private PNG, `privatePngTopVIs1=true`, so ID17 `flipY=true` yields `v=t`. Define `fract(z)=z-floor(z)` so negative/out-of-range UV probes behave correctly. At discontinuity/pixel-edge landmarks, compare away from seams (or define a precise point-sampling tie convention). The oracle's `>=` boundary is a deterministic raster-test convention; real GPU edge coverage rules can differ by a pixel and are not the focus. For a baseline 7-wide source with `W=23`, R=23/7; rotate ±5.153927° and use the same inverse transform. R must be independently supplied from authored/source dimensions, not computed from the implementation's proposed UV outputs.

**ID17 phase landmarks:** unflipped left/right U=(0,3.2384345858); X-flipped U=(3.2384345858,0). Flipped normalized local q=(0,.25,.5,.75,1) yields U≈(3.238435,2.428826,1.619217,.809609,0), wrapped phases ≈(.238435,.428826,.619217,.809609,0). The partial repeat is at the **left** for ID17, while the unflipped companion has it at the right. At each internal integer-U seam, texture restarts but quad geometry and painter state do not. A `uLeft=1,uRight=0` flipped copy, per-tile reflection, or culling/reanchoring to viewport X fails these landmarks. Rotation is applied to geometry before the inverse lookup; U follows the local axis, not screen X.

## Focused invariants / test shape

1. CPU geometry command and independent oracle agree on U/V at all four *rotated* corners and on interior landmark pixels for ±angle, both X flips, both Y flips, and noninteger R. Distinct top/bottom/left/right colors expose sign and orientation mistakes. Include zero-angle as a control, but the test must fail a one-copy stretch.
2. Translate/crop the rotated 23×12 test quad partly outside a 32×24 viewport; expected overlap is derived by inverse transform, not by trimming unrotated destination or calling `ForEachRepeatXTile` with screen X. A corner can be visible even when its unrotated edge is offscreen; the global local-U phase is unchanged by culling.
3. Repeat with camera translation and worldZoom: geometry moves/scales, while authored R and feature order stay fixed. Explicitly test a fractional edge: X flip puts the 0.2384-like remainder at local left.
4. Painter test: draw an opaque/mixed-alpha preceding layer, the repeated quad once, then a distinguishable later translucent/opaque rectangle; verify outside-quad prior color remains, source alpha blends exactly once, and later painter overwrites/blends in the submitted order. Scene order is `stable_sort` descending depth (`LevelScene.cpp:625`) and then serialized `renderer.DrawSprite(draw)` (`:1448`), with dynamic/spray insertions in `:1328-1360`. The renderer must keep this one draw at its existing slot; an offscreen prepass/composite at end-of-frame fails even if UVs are right. Frame 424's ID17 packet draw ordinal 5 is a useful owner-run integration anchor, not a hard-coded renderer condition.
5. This CPU oracle checks mapping/coverage/compositing commands, **not** hardware mip/filter differences, SDL/GL state restoration, or Windows parity. An owner-run GPU test after exclusive ownership is released should confirm ID17's backend choice, R endpoints, transformed corners, ID17 interior ROI and a later-painted control at the same frame; compare to Windows only with matched provenance.

## Implementation recommendation and backend risk

Implement one narrowly dispatched ordinary rotated-repeat branch at `SDLRenderer::DrawSprite`'s existing painter slot, after higher-priority custom effects and before the zero-angle repeat path: `sprite.repeatX && rotationDegrees != 0 && !sourcePtr`, with finite positive dimensions/texture size, scene phase, validated desktop `opengl` context, and successful private repeat texture. Use `SDL_RenderFlush` **before** private GL to drain earlier SDL draws; then one rotated quad with UV `(R,0)` for ID17, GL U/V repeat, correct bottom-first V, source-over blend, tint/opacity, and ordinary shader-like min/mag/mip settings. Restore GL/SDL-visible state and leave the next painter command in sequence. Inspect viewport/scissor under logical-size and resized-window operation; a direct GL draw that ignores SDL's letterbox viewport can misplace/clip corners. Handle allocation/flush failures as explicit fallback/non-parity, not as successful repeat.

The code already has a private full-size NPOT PNG uploader plus complete mip chain (`WobbleMipTexture`, `SDLRenderer.cpp:809-910`), and a flushed rotated private-GL quad example (`DrawRaftMipExperiment`, `:298-355`). These make the approach feasible but do not themselves dispatch ID17. The current SDL-owned texture path (`SDL_GL_BindTexture`, `:359-480`) may expose padded/atlas allocation and UV spans below 1; `GL_REPEAT` on it can repeat padding/neighbors, so do **not** blindly bind it for R>1. `SDL_RenderGeometry` (`:653-680`) offers vertices but no proven repeat/mipmap contract for UV>1 on an SDL-owned texture; segmentation would also introduce rotated seams and filtering discontinuities. A target-texture workaround is not present in the current renderer (`SDL_CreateRenderer` requests acceleration/VSync but not a target-texture capability; no `SDL_SetRenderTarget` path). Pre-rendering tiles to a target would require capability checks, clearing, transparent edge behavior, filtering, one final rotation, and strict same-slot compositing; it is a larger/riskier path and not the bounded recommendation.

No implementation or GPU ownership is claimed here. Preserve all unrelated in-flight changes; the exclusive 415/424 worker owns renderer/build/GPU until released.
