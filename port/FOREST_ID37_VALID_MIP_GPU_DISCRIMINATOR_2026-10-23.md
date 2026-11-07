# Forest ID37 complete-mip physical GPU discriminator — 2026-10-23

## Decision

**A complete mip chain plus mip-linear minification has a small, local final-image effect, not a gross lower-silhouette correction, in the frozen forest-1 Linux frame.** Relative to the byte-verified no-mip baseline, 6,163 of 921,600 pixels change (0.669%), all inside the ID37 lower band `[0,596,1280,720)`; maximum channel difference is **2/255**. The near-black authored ID37 mass remains. This is a valid Linux GL filter discriminator, **not original-Windows parity evidence**: the Windows bound SRV mip count/LOD and matched frame remain unavailable. No shared renderer change is proposed.

## Isolation and reproducible route

Unique ignored evidence: `port/build/forest-id37-valid-mip-gpu-discriminator-2026-10-23/`. The isolated source is a copy of the prior ID37 level-query freeze (`FOREST_ID37_MIP_DISCRIMINATOR_2026-10-23.md`); only its isolated `port/linux/SDLRenderer.cpp` was changed. Source SHA-256 `6d14806e9542f55e5f6628525aebce7e1c723fce358f1ddde03eae71168c7050`; built executable SHA-256 `be97d9757e1534df1e9a3a3fe08d17a8d30a946c0645a1ba5ef101462f4731e1`. The baseline BMP copied from the prior physical-GPU run has SHA-256 `8d85a22bd50aab1ef8adf44702f01567fb432434ab6263e6c249045e088d7bd5`, byte-identical to the independent uninstrumented post-ID21 capture. New valid-mip BMP SHA-256 `5d8297c6b1215b445f51fb76a215cbdcbfc7384bd8c28a23fc8c556275ee34ed`. The frozen forest XML/PNG and endpoint are the same as the prior report.

The one-time ID37-only experiment calls `glGenerateMipmap` on the already-bound SDL GL texture, verifies every required level from **2048×512 (level 0) through 1×1 (level 11)**, then selects `GL_LINEAR_MIPMAP_LINEAR` while drawing ID37 only. Other fractional sprites retain `GL_LINEAR`. The isolated code preserves/restores the sampler parameters around the draw as before. `valid-mip.log` records GL object **225**, pre-generation level 1 **0×0**, all 12 generated level sizes correct, `complete=1`, generator present, `glError=0`, active draw min **9987** (`GL_LINEAR_MIPMAP_LINEAR`), mag **9728** (`GL_NEAREST`), base 0, max 1000, repeat wrap. A max-level setting of 1000 does not require levels beyond the complete chain down to 1×1. The source probe's pre-draw min/mag 9729/9729 are SDL texture state, not the active draw sampler; the active baseline draw uses `GL_LINEAR`/`GL_NEAREST`.

The sole new capture used physical Xorg `:0`, `/dev/dri`, LightDM Xauthority, `deathleak-video-capture:bookworm`, repo read-only and unique writable output. Before capture no competing Death Leak GPU container/process was present. Exact game arguments: `--play Game/XmlFiles/levels/forest_exploration_1.xml --script R:180 --frames 180 --fixed-dt 0.0166667 --capture /out/valid-mip.bmp`. The log reports strict accelerated SDL OpenGL, nouveau/NVE7, Sepia GPU pass, 71 loaded sprites and no missing/unsupported assets; endpoint matches baseline player `(1368.99,3107.3)`, health 25, grounded, `vx=480`. Only wall-clock elapsed varies. After capture no Death Leak GPU container/process remained; lane released.

## Pixel discriminator (post-sepia/HUD, RGB BMP 1280×720)

All boxes are half-open; any changed RGB channel counts as one pixel. `analysis.json` and its `analyze.py` are in the unique evidence directory.

| Region | Changed pixels | Changed bbox | Max channel delta | Mean absolute channel delta, changed pixels |
| --- | ---: | --- | ---: | ---: |
| Full frame | 6,163 | `[0,596,1280,720)` | 2 | 0.550 |
| Onset `[0,580,1280,621)` | 200 | `[448,596,1199,621)` | 1 | 0.538 |
| Bottom 40 rows | 2,046 | `[26,680,1280,720)` | 2 | 0.550 |
| Interior `[100,650,1180,720)` | 3,919 | `[102,650,1180,720)` | 2 | 0.547 |

Changed-pixel counts at rows y596/600/620/650/680/700/719 are **1/2/20/88/51/34/48**. The previous baseline→hide-37 experiment changed **92,357** lower-band pixels versus only **6,163** here, with only 6,032 mip differences overlapping that prior ID37 visible-contribution mask. This is a post-composite/quantized color comparison, not an alpha or pre-sepia measure; a few differences can sit just outside the binary hide mask at antialiased boundaries.

Broad mass persists: lower ROI pixels with all RGB values <16 are **57,440 baseline vs 57,407 valid-mip** (33-pixel net difference); 57,390 are dark in both. At `(640,700)` both baseline and valid-mip are `(4,3,2)`, while hide-37 is `(81,71,51)`; `(100,700)` similarly remains `(4,3,2)` versus hide-37 `(73,63,44)`. At `(640,590)`, all are `(87,75,54)`. The difference extends through sparse texture/edge detail but does not lift or remove the opaque silhouette.

## Limits

This verifies a **generated** Linux GL mip chain and active mip-linear sampling on this one frozen endpoint. Driver mip-generation content is not evidence of the original D3D10 texture loader's exact mip colors, number of bound levels, or sampled LOD. The Windows `BasicLightTexture.fx` source requests mip-linear minification, but no matched Windows bound-resource packet exists; do not infer Windows visual parity or promote this isolated experiment to shared source. The prior baseline and new frame were separate physical-GPU runs but source/frame input and prior baseline hashes are verified. No shared source, checkpoints, commits, or pushes were changed; the pre-existing shared `port/linux/SDLRenderer.cpp` modification was left untouched.
