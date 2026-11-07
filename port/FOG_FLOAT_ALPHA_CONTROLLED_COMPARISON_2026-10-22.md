# Controlled float-alpha fog comparison — 2026-10-22

## Verdict

**Keep the float-alpha fix for source/data-path fidelity, but do not sell it as a material visible improvement at forest-1 frame 300.** An otherwise identical pre-fix binary changes only 7,816 full-scene pixels in the fog's y≥213 region after the fix, each by at most **1/255 per RGB channel** (0.00584/255 mean absolute RGB over the full 1280×720 image, including a small non-fog fluctuation). The fog-only frame changes 15,047 pixels in that region, again at most 1/255 per channel. The corrected fog does contribute strongly to the scene overall; the *incremental float-alpha correction* is subtle at this frame. No matched Windows capture or Jerome visual acceptance exists.

## Control and reproduction

- Built separate complete tracked-source archives at `50ba9e9` (immediate pre-fix parent) and `3160a28` (float-opacity fix) in ignored `port/build/fog-float-alpha-controlled-2026-10-22/{pre-fix,fix}-src/` using the same `deathleak-linux-build:bookworm` image and `make -C port -j2`. Neither shared source nor shared build output was used. The fix archive's rebuilt binary SHA-256 is `8c6279bb99d87eba42fb3afff3a707fa141c5fbb9708d51142acd9b89cb89a8b`, **byte-identical** to the existing current capture binary. Pre-fix binary SHA-256 is `31030639cfade74a611ad6323634a5d4af49ad6a9d5eadf4b39306d3e9fa2861`. Thus the concurrent uncommitted orb-animation edit in the working tree did not enter either comparison binary.
- Before taking the GPU lane, process inspection found Xorg `:0` but no `deathleak-linux` or FFmpeg process. Six sequential fresh Docker runs used the physical X socket, LightDM Xauthority, matching Xauth hostname, `/dev/dri`, SDL X11/OpenGL strict acceleration, dummy audio, forest-1 XML, `--script R:180 --frames 300 --fixed-dt 0.0166667`, and either baseline, hide IDs 119/122, or only IDs 119/122. Exact commands: ignored `port/build/fog-float-alpha-controlled-2026-10-22/capture.sh`. Each log confirms `accelerated=1 software=0`, `nouveau / NVE7`, GPU Sepia, 71 original sprites, zero missing assets, and the same terminal scene/player `(1400.52,3107.54)`, five orbs, grounded. No game/FFmpeg process remained after capture.
- Saved six BMPs/logs, `metrics.json`, `analyze.py`, and 8×-brightened delta previews in that ignored evidence folder. The separately rebuilt fixed captures are byte-identical to all three earlier `port/build/fog-float-alpha-gpu-2026-10-22/` fixed captures, not merely visually similar. This validates the earlier fixed-state comparator without claiming the unbuilt shared working tree is equivalent.

## Pixel measurements

All metrics are exact RGB comparisons of 1280×720 BMPs; mean absolute RGB is the average absolute channel delta on the 0–255 scale. Bounding boxes use `[left,top,right,bottom)`.

| Pair | Changed pixels | Bounding box | Mean abs RGB | Max channel delta |
|---|---:|---|---:|---:|
| Pre-fix vs fix, full-scene baseline | 7,860 (0.853%) | `[0,44,811,720)` | 0.005840 | 65 |
| Pre-fix vs fix, fog-only ID 119/122 | 15,091 (1.637%) | `[0,44,815,720)` | 0.008807 | 8 |
| Pre-fix vs fix, fog hidden | 44 | `[170,44,241,76)` | 0.001301 | 65 |
| Pre-fix baseline vs pre-fix fog hidden | 177,837 | `[0,213,821,720)` | 3.592959 | 74 |
| Fix baseline vs fix fog hidden | 177,863 | `[0,213,821,720)` | 3.597342 | 74 |

The 44-pixel y44–76 delta survives with fog hidden and appears in the fog-only render too, so it is **not attributable to the float-alpha change**; the fixed gameplay endpoint alone does not eliminate every nondeterministic render detail. Restricting to the fog-contribution region y≥213 removes that fluctuation: baseline pre-vs-fix is **7,816 changed pixels, total absolute RGB difference 12,550, max channel 1**; fog-only is **15,047 pixels, total absolute difference 23,609, max channel 1**. Exact per-image hashes and complete metrics are in `metrics.json`.

## Limits and recommendation

This is a controlled Linux physical-GPU comparison at one scripted frame, not a Windows parity test, motion/video evaluation, or proof that all fade times look the same. IDs 119/122 produce a substantial fog-vs-hidden scene delta, but only a near-threshold one-level RGB change from preserving float alpha at frame 300. Keep the narrow fix because it passes authored opacity through to the PixelWobble shader instead of quantizing it to byte alpha, with no observed visual regression; prioritize broader composition/animation acceptance work over further tuning this subtle delta. No shared source, shared build output, checkpoint/status file, commit, or push was changed for this comparison.
