# `sea_2` final bottom-bar source audit — 2026-10-23

## Verdict

The pre-existing final-scene bottom-bar aggregate at `port/linux/main.cpp:1223` is **source-inequivalent and causes a real geometric visual defect** in the SDL path: it draws the bar at **1° rotation**, not vertically flipped. It does **not** make the ordinary SDL sprite transparent: its `opacity = 0` is not used by the ordinary `DrawSprite` copy path. The missing vertical flip has no independently observable pixel effect with the current `frame_top.png`, which is a single row of uniform opaque black pixels. Thus the concrete visual issue is the tilted/clipped lower band (potential slivers/gaps), not a missing band or a visibly wrong texture orientation. This is a source/asset audit only; no runtime capture or visual-parity claim.

## Evidence

- Original `Game/XmlFiles/UI/final_scene_hud.xml:18-34` declares the upper and lower sprites using the **same** `Media\\UI\\frame_top.png`, both `1920×150` and `alpha="1.0"`; upper `vflip="false"`, lower `vflip="true"`, both `hflip="false"`. `Game/src/UI/UISprite.cpp:20-27` reads those flags and alpha; `:82-95` (also `:168-181`) reverses vertical UVs for `m_verticalFlip`. Original lower bar is not rotated.
- The port's separate final-scene branch is `port/linux/main.cpp:1215-25`; its top rectangle `:1222` is `{0,0,1280,100}` and lower rectangle `:1223` is `{0,620,1280,100}`, consistent with the 2/3 mapping of the authored 150-unit heights. This finding does not concern the ordinary gameplay `else` branch at `:1226-39`.
- `port/render/Renderer.h:46-55` orders aggregate fields as `texture, source, destination, tint, opacity, rotationDegrees, flipX, flipY`. Consequently the trailing `0, true` at `port/linux/main.cpp:1223` initializes `opacity=0`, `rotationDegrees=1` (bool-to-float), `flipX=false`, and leaves `flipY=false` by default. The intended `flipY=true` is not set.
- In `port/linux/SDLRenderer.cpp:211-12`, ordinary sprite alpha is taken from `sprite.tint.a`, and `:226-27` computes the SDL flip from the flip flags. Its ordinary fallback at `:266-70` calls `SDL_RenderCopyExF` when rotation is nonzero, passing `sprite.rotationDegrees`. No `sprite.opacity` is applied in that path. The current `Game/Media/UI/frame_top.png` is a 1080×1 RGB PNG; pixel inspection found one unique color, `(0,0,0)` everywhere. Flipping that texture vertically cannot change its pixels, whereas rotating the 1280×100 destination quad changes its screen coverage.

## Minimally scoped source fix (not applied)

Replace **only** the lower draw at `port/linux/main.cpp:1223`, preserving the existing `finalFrame` guard, upper draw, lower rectangle, tint/alpha defaults, and distinct `sea_2` branch:

```cpp
Sprite bottomBar{finalFrame, {}, {0,620,1280,100}};
bottomBar.flipY = true;
renderer->DrawSprite(bottomBar);
```

Here the omitted fields retain `opacity=1`, `rotationDegrees=0`, `flipX=false`, and opaque white tint from `port/render/Renderer.h:50-54`. This matches the original XML flags and restores an axis-aligned lower band. It is safe even if the currently uniform texture is later replaced with vertically asymmetric art. No asset, shader, shared HUD, or ordinary gameplay bar change is required for this specific defect.
