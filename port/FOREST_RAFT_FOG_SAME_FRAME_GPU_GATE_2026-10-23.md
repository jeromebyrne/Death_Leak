# Forest raft/right-branch fog same-frame physical-GPU gate — 2026-10-23

## Verdict

**The current Linux renderer visibly draws both authored fog IDs at the raft/right-branch landmark.** At the same deterministic active forest-1 frame, hiding ID119 removes a broad right/lower haze, while hiding ID122 removes principally left/lower haze. Both together alter 109,994 pixels (50.76%) in the defined raft-exit ground ROI and lower its mean RGB by `(5.888, 5.051, 3.547)` display-code values. A repeat baseline is BMP-byte-identical, so the measured deltas are attributable to the render-only hide controls at this endpoint, not observed capture jitter. **No original-Windows lossless, state-matched frame/sidecar is available; this is a Linux causal attribution, not original-renderer pixel parity.** No source, XML, art, or alpha change is justified by this gate.

## Reproducible capture and controls

- Source HEAD `9ba65c3`; existing current-tree executable `port/build/deathleak-linux`, SHA-256 `71e51bd6c89291503518e094e99873b223e4dba14eedf759d3d3b08ed7f3844c`. No rebuild/source edit was made. All ignored evidence, exact commands, logs, BMPs, a PNG preview, and the pure-Python analysis are under `port/build/forest-raft-fog-same-frame-gpu-2026-10-23/` (`capture.sh`, `analyze.py`). Run those scripts from repo root to reproduce; the capture script uses five sequential Docker runs and no Windows reference.
- Each run enters active `Game/XmlFiles/levels/forest_exploration_1.xml` with `--script R:110 --frames 110 --fixed-dt 0.0166667 --capture ...bmp`. This is the established raft/right-branch composition search endpoint from `FOREST14_LINUX_MATCH_CAPTURE_2026-10-22.md`, not an assertion of exact Steam screenshot time/camera. Endpoint logs agree on player `(808.985,3135.97)`, 3 orbs, 25 health, grounded, no enemies/shots. The full per-frame camera/effect state is not logged; deterministic replay and a zero-delta repeat are the available Linux controls.
- `baseline`, `hide119` (`DEATHLEAK_RENDER_HIDE_XML_IDS=119`), `hide122` (`...=122`), `hide-both` (`...=119,122`) and `repeat` were captured at native 1280×720 RGB. The switch removes selected XML objects only at render; no level content was altered. Every run logs `video_driver=x11 name=opengl ... accelerated=1 software=0 ... mode=strict-accelerated`, `nouveau renderer=NVE7`, GPU `Sepia.fx`, 71 original sprites and zero missing/unsupported assets. `baseline.bmp` and `repeat.bmp` have identical SHA-256 `ce02aaa0178cf0d140289b313be29f6509cbad1944249716ac9cded6dd7eab9d`.

## Measured per-ID framebuffer contribution

Pixel differences are `baseline minus hide variant`, measured after the scene postprocess/HUD. The primary ground ROI is half-open `[100,400,1200,597)`, 216,700 pixels: it covers the lower raft-exit/branch-side ground while excluding HUD and the ID37 near-black bottom shrub silhouette, which prior attribution places at y≥597. Counts treat a pixel as changed when any RGB channel differs; mean signed/absolute RGB deltas average across **all** ROI pixels, including zero-delta pixels. Bounding boxes are half-open screen coordinates.

| Hidden at same frame | Full-frame changed pixels / bbox | Ground ROI changed pixels | Ground ROI mean signed RGB delta | Ground ROI mean absolute RGB delta |
|---|---:|---:|---:|---:|
| ID119 | 185,416 / `[201,430,1280,720)` | 74,255 (34.27%) | `(4.350,3.736,2.622)` | `(4.361,3.738,2.623)` |
| ID122 | 131,525 / `[0,449,663,720)` | 47,023 (21.70%) | `(1.447,1.237,0.870)` | `(1.515,1.298,0.916)` |
| Both | 266,276 / `[0,430,1280,720)` | 109,994 (50.76%) | `(5.888,5.051,3.547)` | `(5.951,5.099,3.582)` |
| Repeat baseline | 0 / none | 0 | `(0,0,0)` | `(0,0,0)` |

The baseline ROI mean RGB is `(70.478,60.663,43.051)` versus `(64.589,55.612,39.504)` with both hidden. Spatially, ID119 contributes 51,562 changed pixels and `(5.857,5.035,3.529)` signed mean RGB in right-lower `[600,400,1200,597)`; ID122 contributes only 1,128 and `(0.017,0.015,0.011)` there. Conversely ID122 contributes 51,353 changed pixels and `(5.735,4.908,3.451)` in left-lower `[0,450,500,597)`, against ID119's 8,422 and `(1.232,1.053,0.742)`. The IDs overlap and blend in draw order, so the single-ID framebuffer deltas are **conditional marginal effects**, not additive intrinsic texture/alpha values. A broader prior-study-compatible ROI `[0,500,1280,700)` has mean absolute RGB deltas ID119 `(7.949,6.804,4.787)`, ID122 `(4.727,4.057,2.865)`, both `(13.066,11.181,7.891)`; it includes bottom silhouette and is not the primary raft-exit measure.

## Alpha interpretation and parity boundary

XML `forest_exploration_1.xml:1629-1665` authors ID119 `Media/fog.png` alpha `0.5`, ID122 alpha `0.6`, both `kGroundFront`, both `FadeInTime=6`. Current Linux `LevelScene.cpp:884-909` scales each sprite's alpha by `scrollElapsed/6` during fade, matching the original `ScrollingSprite.cpp:22-55` rule. **If** both receive all 110 fixed 1/60-s updates from zero, the expected pre-quantization sprite alpha at this endpoint is approximately ID119 `0.15278` and ID122 `0.18333` (8-bit tint alpha about 39 and 47). These are **source-derived expectations, not directly logged GPU alpha measurements**. The captured BMP is opaque postprocess RGB; its pixel deltas cannot uniquely recover per-fragment texture alpha, blending, occlusion, or an original-Windows alpha.

The saved Steam screenshot 14 is a compressed, unmatched landmark reference with unknown game-frame time, camera/view, player/orb state and effect phase. The original-Windows lossless post-HUD frame plus synchronized same-ordinal sidecar requested in `FOREST1_MATCHED_FRAME_CAPTURE_PACKET_2026-10-23.md` is not present here. Consequently the Linux baseline cannot be graded as Windows pass/fail or used to tune fog brightness. Next parity gate: acquire that original packet, reproduce/register its actual state and static landmarks in Linux, then compare the same ground ROI while retaining these baseline/hide-ID controls. Until then, preserve authored fog/fade.

## Scope/release

Only this report was written outside ignored `port/build/` evidence. Dirty `port/linux/SDLRenderer.cpp`, `port/tests/integration.sh`, shared `CHECKPOINT_STATUS.md`/`PORT_WORK_STATUS.md`, and all unrelated changes were left untouched. No software change, commit, or push. All five named capture containers exited; no Death Leak capture container remains, releasing physical Xorg `:0`/NVE7 ownership.
