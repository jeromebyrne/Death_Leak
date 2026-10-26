# Forest cave-blur segmented GPU replay — exact-pixel rejection

## Provenance and bounded candidate

Exact remote feature base `4d228a43d76565215dd73be4c61ffd7be60b8420`; isolated clean detached `/tmp/deathleak-segmented-gpu-20261026`. Primary checkout's divergent local `a5e39ac` and unrelated dirty files were untouched except append-only `PERFORMANCE_CHECKPOINT_STATUS.md`. Source image `Game/Media/levels/waterfall_bottom/cave_background_blur.png` is 1750×1313; the five ordinary, linear-sampled Forest cave-blur painter quads are IDs 53,54,55,60,61. Their **actual** XY/UV client-memory arrays come from authenticated physical NVE7 apitrace calls 6686/6684, retained in `alpha-zero-trim-evidence/`. The earlier source oracle certified 3×3-neighbor alpha-zero strips `[1600,1,1749,1312)` and `[1,1213,1600,1312)`; affine visible area at the retained pose was 40.00%, not a fragment count or safe-pixel proof.

A standalone GL2 controlled replay harness (`port/tools/cave_segmented_gpu_replay.cpp`) loaded the full PNG RGBA, restored source top-row UV orientation, rendered the 30-vertex original and a painter-ordered six-rectangle-per-quad replacement excluding precisely those two guarded strips. Both variants used the same offscreen 1280×720 RGBA8 pre-Sepia target, GL linear/clamp sampling, source-alpha blend and source-white vertex color (`blob_call6685.bin` confirms all 30 colors are `ffffffff`). The target was read back byte-for-byte. Nine relative transforms used base, X −43/+200, Y −100/+100, zoom .9/1.1 around screen center, and shake (+3,−3)/(−3,+3). These transforms are **analytic perturbations of the one retained actual XY array**, not nine independently emitted SDL game frames. The replay therefore tests a concrete segmentation mechanism but cannot certify the SDL runtime transform domain.

## Physical result

The final replay exposed only `/dev/dri/card1` and `renderD128`, bound authenticated Xorg `:0`, and logged `GL vendor=nouveau renderer=NVE7`, `glerr=0` for every draw. The first dual-device attempt selected Intel despite `DRI_PRIME=1`; its output was discarded and overwritten. Source, full readbacks, SHA-256 and per-pose comparison are retained under `/srv/openclaw/workspace/death_leak_analysis/segmented-gpu-evidence/` (`replay-nve7.log`, `comparison.json`, `pose*-base.rgba`, `pose*-seg.rgba`).

| Pose | Changed RGBA pixels | Difference bbox (GL readback coordinates) |
| --- | ---: | --- |
| Base | 1,370 | (124,2)–(1277,149) |
| X −43 | 1,372 | (81,2)–(1276,149) |
| X +200 | 1,123 | (324,2)–(1279,149) |
| Y −100 | 1,545 | (75,0)–(1277,249) |
| Y +100 | 650 | (281,0)–(1275,49) |
| Zoom .9 | 1,375 | (140,0)–(1279,171) |
| Zoom 1.1 | 1,606 | (92,0)–(1279,126) |
| Shake (+3,−3) | 1,377 | (123,0)–(1279,152) |
| Shake (−3,+3) | 1,364 | (121,0)–(1273,146) |

Base-pose channel differences reached 5/255 and occur in non-background image pixels; a zero-alpha source guard does not prevent changed float32 interpolation/rasterization on retained triangles after subdivision. The replacement also expands 30 to 180 vertices. The standalone path does not include the full scene/SDL queue, so this is a **rejection discriminator**, not a quantified gameplay visual delta or speed result. No exact-safe runtime geometry candidate exists from this design. A full SDL pre-Sepia capture cannot rescue a candidate already non-identical in controlled NVE7 rendering; no production B, B build, serial production A/B/A, timing, binds/draws or VRAM improvement is claimed. Default atlas coverage remains 23/896. No 60-FPS, sustained/thermal, 8K/high-Hz or release acceptance.

## Distinct next task

Move off cave-blur segmented geometry. Investigate the measured first-entry Waterfall-top `cave-background.png` decode/upload stall as a separate transition lane: retain authored fade and cache IDs, prototype *asynchronous decode only* in an isolated source build, prove main-thread upload frame pacing and exact transition pixels on physical NVE7 with serial A/B/A, then keep or reject. Earlier traces measured ~30.3 ms decode plus ~21.1 ms upload for that texture and a 90–130 ms top-entry update spike; moving synchronous work into a partially visible fade is not assumed safe. This is distinct from a fifth cave-blur CPU oracle or unmeasured atlas page.
