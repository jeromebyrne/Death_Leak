# Waterfall-top V autoscroll implementation — 2026-10-22

Scope: eight authored `waterfall_top.xml` `waterfall_4.png` ParallaxLayers (IDs 6, 7, 10, 13, 14, 15, 16, 31). No XML/art change and no physical GPU/X11/FFmpeg capture in this lane.

## Implementation

- `LevelScene` now reads `autoscrolly` and `autoscrollyspeed`, retains a separate fractional V phase for each sheet, and advances it exactly once per `UpdateVisuals`. At the fixed 1280×720 target the legacy `Scale` multiplier is 720/1080; a 1/60-s update advances V by authored speed × 2/3 × .001: 7→.00466667, 9→.006, 11→.00733333. The existing `Draw` uses that phase in preference to camera-Y parallax, as legacy `ParallaxLayer::Draw` does. Quad placement, depth, alpha and textures are unchanged. Draw is state-free, including diagnostic redraws.
- Cadence choice: this port advances using the current `UpdateVisuals(dt)` before Draw. Windows' Draw reads the *preceding* completed update delta, so first frame and a delta/time-modifier change can differ by one frame. Steady-state rate is equal. This bounded slice does not add an extra clock solely to reproduce that lag.
- The SDL wrapped-UV path now reverses **source tile order** for locally flipped copies on both axes. Previously the seam positions/texel order of vertically flipped split tiles did not represent a single whole-quad flipped texture. Tile alpha remains the sprite's texture alpha modulation. Zero-length tiles are omitted at exact or quantized integer phases.

## Verification

- Docker authored-scene capture regression: **PASS**. It loaded the real waterfall-top XML, captured all eight ID-filtered draws at a fixed camera, checked per-layer 7/9/11 speed and 2/3 scale after 1, 60 and 600 updates, stable destination/opacity/tint/flip, no repeated-Draw phase advance, and camera-Y independence.
- Docker SDL software pixel regression: **PASS**. An asymmetric eight-row texture exercised normal and flipped wrap at phase 0, .25, .75, just before and just after 1, checking source row order, seam placement, no zero-length copies and 50%-alpha output. This is a renderer-mapping regression, **not** physical-GPU visual evidence.
- Existing authored forest camera-parallax speed regression: **PASS**. Docker `make -C port -j2`: **PASS** (pre-existing TinyXML parser warnings only).

## GPU follow-up (separate lane)

Run active `--play Game/XmlFiles/levels/waterfall_top.xml --fixed-dt 0.016666667` under the real accelerated SDL/OpenGL GPU renderer on physical X11, from identical starting state/input for `--frames N --capture early.bmp` and `--frames N+60 --capture late.bmp`. Keep camera/player nearly fixed over the shaft/bridges/orbs; `--camera-script` simulation is invalid because it skips visual updates/render. First isolate IDs `6,7,10,13,14,15,16,31` with `DEATHLEAK_RENDER_ONLY_XML_IDS`, then capture unfiltered composition. Verify flow direction, distinct 7:9:11 rates, stable quad edges/alpha, and no split seam/flicker; request human review of real-GPU frames/video. The Steam still and software pixel probe cannot establish motion parity or visual acceptance.
