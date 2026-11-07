# `sea_2` final bottom bar — patch preflight (2026-10-23)

## Decision

The defect remains in the live `port/linux/main.cpp` final-scene branch at inspection: its lower `finalFrame` aggregate ends `..., {255,255,255,255}, 0, true}`. This is separate from the now-present ordinary-gameplay 50-unit bars in the `else` branch. Fix the final-scene lower draw only after the current `main.cpp`/GPU owner releases it. This is a source-backed geometric correction, not a demonstrated pixel-parity result; no render was run here.

## Exact edit packet

At `port/linux/main.cpp:1223`, replace the one lower draw inside the existing `if (finalFrame)` with:

```cpp
Sprite bottomBar{finalFrame, {}, {0,620,1280,100}};
bottomBar.flipY = true;
renderer->DrawSprite(bottomBar);
```

Leave the immediately preceding top draw `{0,0,1280,100}`, the `finalFrame` guard, film/title draws, scene postprocess, and the ordinary-gameplay `else` branch unchanged. No renderer, XML, asset, or shared HUD edit is needed. Re-read the live branch before applying because other owners are active; do not format or commit their unrelated changes.

Why this exact form: `port/render/Renderer.h:46-58` orders fields `texture, source, destination, tint, opacity, rotationDegrees, flipX, flipY` (followed by newer fields). The current trailing `0, true` sets opacity=0 and rotationDegrees=1, **not** flipY; missing fields default to flipX=false and flipY=false. The named assignment restores opacity=1, rotation=0, flipX=false, flipY=true, tint opaque white. `port/linux/SDLRenderer.cpp:211-12,226-27,266-70` uses tint alpha and flip flags for ordinary copies, choosing `SDL_RenderCopyExF` for the current 1-degree rotation; it does not apply `Sprite::opacity` in this path. Thus the existing draw is tilted rather than made transparent.

Original contract: `Game/XmlFiles/UI/final_scene_hud.xml:18-34` declares both 1920×150 `frame_top.png` bars at alpha=1; only the lower is `vflip=true`, and neither is rotated. `Game/src/UI/UISprite.cpp:20-27,82-95` reads and applies the vertical UV flip. The port's two 1280×100 rectangles are the 2/3-scale positions. The present `Game/Media/UI/frame_top.png` is a single uniform black row (per `port/SEA2_FINAL_BAR_SOURCE_AUDIT_2026-10-23.md`), so vertical flip cannot by itself change current pixels. The 1-degree rotation can change bar coverage near screen edges. Keep the flag for source parity and future asymmetric art, but do not claim that a black screenshot proves it.

## Focused acceptance for the future implementer

1. **Source/command gate:** The final `sea_2` lower sprite must reach `DrawSprite` with `texture=finalFrame`, full-source rectangle, destination `{0,620,1280,100}`, tint RGBA `{255,255,255,255}`, `opacity=1`, `rotationDegrees=0`, `flipX=false`, `flipY=true`. Verify via direct Sprite inspection or a small command spy if the main-loop seam is made injectable. In the same review, confirm the upper final bar stays `{0,0,1280,100}` with both flips false and the ordinary `else` branch retains its separate approximately 33⅓-pixel bars. Merely checking black output does not validate flipY.
2. **Runtime visual gate (GPU owner only):** On a `sea_2` final-scene frame at 1280×720, confirm uninterrupted, axis-aligned black coverage at interior samples around y=50 and y=670 and at bottom-edge/interior x positions; inspect the former lower-band top edge near y=620 and left/right ends for the pre-patch sliver/tilt. Check film/title still layer as before. Compare before/after in the same state if a pre-patch capture exists. A black band alone is insufficient to prove source UV orientation.
3. **Regression boundary:** In ordinary gameplay, retain the 50-authored-unit top/bottom bands and their ordering over film and under widgets. Do not reuse the final-scene 150-unit dimensions there. `finalFrame` failure should continue to skip both final bars without changing title/film behavior.

No new mirrored unit test is required for this single aggregate repair; the source/command gate detects the actual wrong-field regression, while the runtime gate detects the geometric effect. If an existing test harness can expose final-scene draw commands cheaply, encode item 1 there, rather than adding a test that only searches for the implementation text.

## Ownership and scope release

This preflight wrote **this report only**. No source, shared status, test, build, Docker, GPU/display use, or commit was performed. The current GPU/main.cpp owner retains exclusive implementation and capture scope; this read-only `sea_2` audit scope is released on report completion.
