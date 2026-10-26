# Waterfall-top per-painter GPU/order gate — no accepted fusion

Clean detached source `e049ddb4f57306faa4e4e7aea62bcd3008c42685` in `/tmp/deathleak-painter-gpu-20261026`; primary unrelated dirty work untouched. Isolated production Docker `make -C port -j2` passed (only existing TinyXML warnings). Physical Xorg `:0`, authenticated `DISPLAY=:0`, hardware nouveau/NVE7. The existing exact-ref three-frame Waterfall-top apitrace recorded accelerated=1/software=0/vsync=1, output 1280x720, and was replayed serially four times with `apitrace replay --pgpu -v`. The extra diagnostic-only ID 117 and ID 121 filtered traces were captured from the freshly built exact source and verified their distinct last-frame opacity uniforms. No production code was changed.

## Painter identity, uniforms and ordering

`LevelScene::LoadLevel` stable-sorts by descending depth and `Draw` visits equal-depth objects in authored XML order. The source XML and trace map the two `kGroundBack` waterfall_4 quads to IDs 13 then 14 (same private texture 387, effectTime 0.03333334, speedMod 10, intensity 0.005, spriteAlpha 0.3, textureSpan 1,1 on the last trace frame). They are consecutive GPU draws 9305/9356, with no intervening draw. The four `kGroundFront` fog.png quads are IDs 117, 118, 119, 121 in stable XML order. Their private texture is 389; time 0.03333334, speedMod 0.1, intensity 0.1, span 1,1. Last-frame spriteAlpha is respectively 0.015, 0.005, 0.005, 0.004166667. Filtered single-ID physical traces independently confirmed ID117=0.015 and ID121=0.004166667. They are consecutive GPU draws 9921/9972/10023/10074, but opacity differs. The two sheets and fog run are separated by 14 other draw calls; the last fog and Sepia are separated by nine. Sepia program 44 draws full output after scene-target switch (call 10422), before HUD. Thus merging all six or folding Sepia into scene drawing violates painter/compositing order. In particular, nonlinear Sepia after alpha compositing is not per-sprite distributive.

## Physical per-draw GPU timer samples

Durations below are milliseconds for each last-frame `glBegin`/quad event from four **serial replays of the same recorded frame**, not four independent gameplay runs. Profiling inserts GPU timer queries; it excludes per-sprite CPU state setup and is not query-off frame pacing. Draws remain in painter order.

| Painter | Call | Replay 1/2/3/4 ms | Median ms |
| --- | ---: | --- | ---: |
| ID13 waterfall sheet | 9305 | .269/.229/.229/.229 | .229 |
| ID14 waterfall sheet | 9356 | .325/.273/.272/.273 | .273 |
| ID117 fog | 9921 | .108/.106/.103/.108 | .107 |
| ID118 fog | 9972 | .460/.456/.463/.456 | .458 |
| ID119 fog | 10023 | .456/.449/.442/.451 | .450 |
| ID121 fog | 10074 | .449/.452/.447/.449 | .449 |
| Sepia full screen | 10422 | .891/.889/.886/.913 | .890 |

Six wobble draws sum to 2.068/1.966/1.955/1.966 ms in those replays. The three later replays agree closely; first-replay warmup shifts the two sheet durations. These values measure fragment/raster work that an exact-pixel batch would still perform; summing them is not a predicted gain. The earlier live query-on 180-update fog and Sepia studies found nonzero aggregate spans but no removable pass; this per-ID profile narrows attribution and exact order rather than superseding those controls.

## Decision and next discriminator

**No shader/pass or batch candidate accepted.** Four fog quads share texture and most uniforms but have different opacity and overlapping alpha blends. A one-call batch would require a new per-vertex alpha path with proof of identical interpolation and output; existing PixelWobble is per-draw uniform. The two adjacent sheets have identical uniforms and texture, so a two-quad batch could save one recurring private-GL setup/draw submission, but not their ~0.50 ms combined fragment work; prior one-draw ordinary branch batch and five-uniform-location cache gave no repeatable frame benefit. This trace does not establish a large enough removable GPU mechanism to justify changing the shader/pass. Sepia already has one draw and zero framebuffer-copy fallback; its target sampling cannot be omitted while preserving output. No safe skip/occlusion mask was proven for these nonzero-alpha quads.

Because there is **no B implementation**, strict candidate A/B/A full-pixel, calls and timing acceptance gates are **not met and no gain is claimed**. The ID-filter captures are diagnostics, not parity controls. A future two-sheet batch must first prove same-source/sampler/geometry-state batching in a focused fixture, then synchronized full-frame physical A/B/A and query-off p50/p95/p99 with recurring GL call reduction; otherwise reject. Default atlas coverage remains **15/896; 881 unatlased**, not 881 proven ineligible. No 8K, high-refresh, 60-FPS or release acceptance.

Local evidence: prior exact-ref `/tmp/deathleak-co-resident-evidence/waterfall-active.trace` and `.glcalls`; new `/tmp/deathleak-painter-pgpu{2,3,4,5}.txt`, `/tmp/deathleak-painter-evidence/id117.*`, `id121.*`, and `/tmp/deathleak-painter-build.log`.
