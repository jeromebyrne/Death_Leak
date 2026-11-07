# Private Forest-1 painter-command CPU discriminator — 2026-10-23

## Verdict

**Negative source-model gate.** An isolated Python 3/stdlib XML-and-formula check passed for IDs 15/17/19/21/22/37 at the previously measured Linux `R:180` camera `(1248.98621, 3400)`, 1280×720, scale 2/3, normal object alpha/no diagnostic hide filter. It found no source-proven order, draw-parameter, or alpha mismatch requiring a patch. This is a deterministic reconstruction of `LevelScene::Load/Draw`, **not** a spy of actual runtime `DrawSprite` calls, a GPU/image comparison, or Windows visual parity. At this particular camera ID15 is culled; its pre-cull command is modeled but it contributes no pixel. No single-frame claim about ID15's right-exit appearance follows.

## Reproducible CPU check and result

The executed read-only Python 3 heredoc used `xml.etree.ElementTree` to enumerate the six authored XML siblings in file order; normalized texture separators; mapped depth values from `LevelScene.cpp:28-40`; stable-sorted descending depth as `LevelScene.cpp:624`; applied `LevelScene.cpp:1341-1411`'s world-follow, depth parallax, 2/3 viewport, flip, rotation, repeat and UV rules; then applied `ViewportCull.h`'s rotated AABB. Assertions passed for order `[19,21,17,22,15,37]`, all `effectlighttexture`, tint alpha `[255,255,255,255,255,242]`, IDs 19/21/17/22/37 admitted and ID15 culled. No source/build or shared test was changed. The check can be reproduced by parsing `Game/XmlFiles/levels/forest_exploration_1.xml` and computing for each ID:

```text
sort key = -depth, stable XML sibling order
dst.x = 640 + ((followX ? cameraX : worldX) - cameraX) * layerX * 2/3 - width/3
dst.y = 360 + (cameraY - (followY ? cameraY : worldY)) * layerY * 2/3 - height/3
dst.w/h = authored width/height * 2/3
tint.a = floor(clamp(XML alpha,0,1)*255 + 0.5)
ID37 uvShiftX = cameraX * 0.000690
```

| Painter ordinal among selected IDs | ID / XML sibling index / depth | Texture (under `Game/`) | Destination x,y,w,h px (rounded .001) | Command flags / fate |
| ---: | --- | --- | --- | --- |
| 1 | 19 / 19 / 5500 | `Media/levels/waterfall_top/ground_blur.png` | −2156.473, 401.429, 4902.746, 682.667 | repeatX, flipX, SDL flipY; admitted |
| 2 | 21 / 21 / 5500 | `Media/objects/tree_strip_dark.png` | −664.300, −269.947, 1323.333, 825.333 | native X (no repeat), SDL flipY; admitted |
| 3 | 17 / 17 / 5000 | `Media/levels/waterfall_top/ground.png` | −2326.374, 464.827, 4902.746, 682.667 | repeatX, flipX, SDL flipY; admitted |
| 4 | 22 / 22 / 5000 | `Media/objects/tree_strip.png` | −1611.371, −166.187, 3364.740, 825.333 | repeatX, flipX, SDL flipY; admitted |
| 5 | 15 / 15 / 1750 | `Media/sun_shaft_2.png` | 1501.333, 318.000, 341.333, 341.333 | rotation +84.225° after authored −1.469999 rad and inverse SDL V; **culled** by rotated AABB |
| 6 | 37 / 37 / 1000 | `Media/levels/forest_exploration_1/shrubbery.png` | −42.667, 589.333, 1365.333, 341.333 | followCameraX, UV X shift +0.861800, one full span/no repeat, SDL flipY; admitted |

All but ID37 have authored alpha 1/tint byte 255; ID37 has XML alpha .95/tint byte 242. `BasicLightTexture.fx:53-59` multiplies sampled texture alpha by object alpha once. Linux's ordinary SDL copies set texture alpha modulation to tint byte (`SDLRenderer.cpp:202-225`), and the ID37 fractional-parallax `glColor4f` path likewise uses tint alpha/255 once (`SDLRenderer.cpp:439-440`); `draw.opacity=.95` remains command metadata and is **not** a second multiplier in that current branch. Thus the quantized Linux factor for ID37 is 242/255≈.949020 versus original unquantized .95, a known byte-rounding difference, not a demonstrated visual defect. Texture RGBA/sample, lighting, blending, later objects and sepia can still affect pixels. IDs 19/21 and 17/22 share depths, so stable XML order puts the occluding strips later at those painter slots; this alone does not assert last-writer pixels.

## Evidence limits and next gate

The camera is tied to the saved physical-GPU `R:180` baseline by the byte-identical GDB capture in `FOREST_MIDDLE_BAND_CURRENT_FRAME_GPU_CAUSAL_2026-10-23.md`; this task did **not** invoke that GPU path. The script evaluated source-model commands rather than using a live recording renderer, and cannot detect a runtime-only divergence from the modeled code. No locally available lossless original-Windows *same-render-ordinal* post-HUD Forest-1 frame plus atomic camera/view/projection, effect clock, player/HUD/object state packet exists (`FOREST1_MATCHED_FRAME_CAPTURE_PACKET_2026-10-23.md`). Obtain that packet, then make a separately owned same-state Linux capture/actual command trace under the exclusive render owner. For ID15's pale field, capture a second matched right-exit frame where its rotated quad is admitted; the `R:180` state cannot judge it. Compare ID19/17 exposed alpha-edge profiles, ID21/22 overdraw, ID37 lower crop and ID15 right-exit crop only after state/landmark registration. Escalate only a measured, object/path-local discrepancy; otherwise retain this negative gate.

## Ownership and release

This lane wrote only this uniquely named private report. It did not touch `main.cpp`, `SDLRenderer.cpp`, shared checkpoints/status, tests, source, build, Docker, Xorg/GPU, images, or another owner's work. **Forest painter-command CPU lane and report ownership are released.**
