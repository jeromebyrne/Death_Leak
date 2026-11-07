# Forest ground-haze physical-GPU discriminator — 2026-10-22

## Verdict

The current port **does draw the authored ground haze**. In a controlled active forest-1 frame, XML `ScrollingSprite` IDs **119 and 122** (`Media/fog.png`) brighten the lower scene; ID119 is the larger and wider contributor, while ID122 is concentrated at lower left. The strong visual contrast between the current-HEAD 1-second gameplay still and Steam screenshot 14 is plausibly affected by **the authored six-second fade-in**: a later current-port capture shows more than twice the fog-contribution mean in the lower-ground sample. This is a demonstrated Linux layer/time discriminator, **not proof of the Steam screenshot's exact simulation time or Windows pixel parity**. No source/XML/art defect was isolated; no such file was changed.

## Capture control and provenance

- Worktree HEAD `c1725ee18315ab8a6192f8be8b227e58386206d7`; binary SHA-256 `a10f8b81dd60c2897dbd3a06a6eb2ce5405072d1d296dd99f9e0ca8fbbd1ce29`. The previously verified post-all-flip 10-second gameplay and 1-second still are in `port/build/post-all-flip-gameplay-2026-10-22/`. Steam reference is `../store_media/screenshots/14.jpg` (1920×1080 JPEG), a scene/location reference but not a camera/state/time-matched Windows frame.
- New ignored evidence: `port/build/forest-haze-gpu-discriminator-2026-10-22/`, including exact `capture.sh`, baseline/hide119/hide122/hide-fog/only-fog/hide37 120-frame BMPs, logs, PNG previews, a byte-identical baseline repeat, and baseline/hide-fog 360-frame BMPs/logs. All captures launch directly into `forest_exploration_1.xml`, then use `--script B:120,F:99700 --frames 120` (or 360) and `--fixed-dt 0.0166667`; capture occurs only after the in-level update sequence. The baseline and all diagnostic endpoints report the same 120-frame player position `(888.986,3116.18)`, four orbs and 25 health. The 360-frame pair report `(920.525,3103.94)`, four orbs and 25 health. Wall-time `elapsed` in the logs varies across runs, so it is not used as the controlled time variable; scripted frame count is.
- Runs were sequential on physical Xorg `:0` via `/dev/dri`, read-only LightDM Xauthority and SDL X11/OpenGL. Every run logs `accelerated=1 software=0 ... mode=strict-accelerated`, `nouveau / NVE7`, initialized GPU Sepia.fx, 71 sprites, zero missing/unsupported assets and forest-1 level audio entry. The diagnostic switch only hides or isolates XML IDs at draw. The baseline repeat is **pixel-identical** (SHA-256 `bdfb134e...` for both BMPs), bounding capture-to-capture render noise at this endpoint to zero observed pixels. No menu/setup still was used.

## Same-frame attribution (1280×720 RGB, frame 120)

| Diagnostic versus baseline | Changed pixels | Difference bounding box `[x0,y0,x1,y1)` | Mean absolute RGB delta in ground ROI `x=0..1279,y=500..699` |
|---|---:|---|---|
| hide ID119 | 233,631 | `[120,246,1220,720)` | `(7.50,6.42,4.51)` |
| hide ID122 | 73,598 | `[0,482,456,720)` | `(4.81,4.13,2.91)` |
| hide both fog IDs | 274,460 | `[0,246,1220,720)` | `(12.48,10.68,7.53)` |
| hide ID37 | 90,046 | `[0,597,1280,720)` | `(5.85,5.05,3.58)` |
| baseline repeat | 0 | none | `(0,0,0)` |

With both fog IDs present, ground ROI mean RGB is `(50.13,43.19,30.74)`; with them hidden it is `(37.72,32.56,23.25)`. Thus the fog is **additive-looking light haze over the composite**, not an absent layer. ID37 affects only y≥597 at this frame and removes dark shrubbery when hidden; it cannot account for the broader haze that begins around y246. The `only-fog` BMP confirms the fog textures' visible low-ground footprint, but it is not a replacement composite or an original-renderer reference.

## Time discriminator and source contract

At frame 360 on the same scripted route, baseline-versus-hide-fog changes 242,032 pixels within `[0,258,1183,720)`; ground ROI mean absolute delta rises to `(28.27,24.22,17.01)`, versus `(12.48,10.68,7.53)` at frame 120. The exact per-pixel geometry changes with scrolling/camera/effect time, so this is a **within-port temporal contrast**, not an alpha ratio or matched screenshot difference. It establishes that a later active frame can look substantially hazier without changing source or art.

The shipped XML at `Game/XmlFiles/levels/forest_exploration_1.xml:1625-1665` authors both fog sprites at `kGroundFront`, `Media/fog.png`, `effectpixelwobble`, original alphas 0.5/0.6, and `FadeInTime="6.000000"`. Windows `Game/src/GameObjects/ScrollingSprite.cpp:22-55` advances its timer and sets alpha to `originalAlpha * currentTime / FadeInTime` before full opacity. Linux `port/game/LevelScene.cpp:750-773` uses the corresponding timer/fade rule. Thus a one-second *recording timestamp* after level entry cannot be treated as a fully faded original reference. Conversely screenshot 14 provides no game-frame time, so the six-second explanation remains a plausible contributor, not a proven sole cause. JPEG/letterboxing, camera/player/HUD state and shader phase also differ.

## Next action and GPU ownership

For a real parity verdict, obtain a lossless original-Windows forest-1 frame at the bridge/tree landmark with camera/player coordinates, viewport, frame-since-level-entry and effect/weather phase; capture Linux at matching state and inspect IDs 119/122 after the authored fade. The current 1-second and Steam stills should not drive a brightness tweak or hiding ID37. Keep the authored fog and six-second fade unchanged unless that matched control reveals a specific defect.

**GPU lane released:** all named Docker captures exited, no Death Leak game/FFmpeg capture remains, and this worker will not use the physical display further. `CHECKPOINT_STATUS.md`, `PORT_WORK_STATUS.md`, CPU follow-lag report, source, XML and art were untouched. No commit/push was needed for this diagnostic report.
