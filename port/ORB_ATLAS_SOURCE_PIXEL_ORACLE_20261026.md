# Orb atlas source-pixel/filter oracle — 2026-10-26

## Base and bounds

Clean detached exact remote feature base `3f3aa2b55345bacd074e4ebe1dd277c174504370` in `/tmp/deathleak-orb-oracle-20261026`; primary checkout's unrelated dirty work was not used or modified except append-only checkpoint. No competing local Death Leak game/build/apitrace process existed at preflight. The rejected physical NVE7 A(off)/B(on)/A(off) at fixed Forest frame 3 had 22 changed RGB pixels, each at most 1 LSB; controls matched. This investigation does not reuse that rejected B as acceptance.

The source-pixel oracle is `python3 port/tools/orb_atlas_source_pixel_oracle.py --atlas-dir DIR`. It checks exact RGBA source reconstruction and samples the vertically flipped 167×255 ordinary orb sprite minified to a 32×49 physical-pixel footprint across all ten animation frames and four subpixel camera phases (0, .25, .5, .75): 40 states. It models float32 endpoint normalization/interpolation and bilinear RGBA source pixels. This is a bounded CPU diagnostic, not an exact emulation of NVE7 raster interpolation, fixed-point stages, blend or postprocess.

## Proven defect and repair

The old full-RGBA page is 171×2590 (96.15% source-rectangle occupancy, within 4096). Its source rectangles reconstructed byte-exactly, but the two-pixel edge extrusion used Pillow `resize` without a resampling mode. Pillow's default bicubic filtering changed **4,790 of 16,880 edge-padding texels** relative to their source edge texels. Example: frame `1.png`, source last-row RGB at x=51 is `(116,116,116)` while atlas first extruded row is `(127,127,127)` at alpha 2. Thus exact source reconstruction did not establish filter-footprint equivalence. The new packer explicitly uses nearest-neighbor edge replication and verifies all four padded edges. The repaired page has 0/16,880 edge mismatches, remains 171×2590, and all ten source rectangles reconstruct byte-exactly. New verification rejects the old page with `non-exact atlas edge extrusion: Game/Media/orb/1.png`.

## Residual and routing decision

The bounded float32 source-pixel surrogate changed from 283 RGBA samples in 22/40 states (max channel delta 58, mostly low-alpha edge RGB) to **32 samples in 20/40 states (max 1 LSB)** after exact padding. Atlas normalized coordinates use page height 2590 rather than source height 255; the oracle measures up to 0.0002594 source texel coordinate divergence under its float32 interpolation. This residual is a distinct UV/raster precision mechanism consistent in scale with, but not proven to be the cause of, the previous physical 22 RGB pixels. It is unsafe to infer exact NVE7 parity or that padding alone repairs the rejected physical B. No orb routing was integrated or enabled, and no repeat of the same failed physical atlas was run without an exact-safe mechanism. Default atlas coverage remains 15/896.

Next distinct candidate: a pixel-center/UV quantization-aware sampling path whose physical NVE7 single-orb oracle proves source-texel coordinates and composited pixels exact for multiple animation ordinals and camera subpixel offsets, then serial full-frame A/B/A. If normalized shared-page sampling cannot be made exact, retain dedicated minified flipped orbs as a documented exception. Do not claim draw-call savings from atlas identity alone.

## Verification and limits

Reproducer: `python3 port/tools/atlas_pack.py --full-rgba --output /tmp/deathleak-orb-oracle-fixed Game/Media/orb/*.png`, then run the oracle on old `/tmp/deathleak-atlas-orb-probe` and repaired page. Results are `/tmp/deathleak-orb-oracle-{results,fixed-results}.json`. Isolated production Docker build result and exact scoped commit/ref are recorded in the checkpoint. No new physical B/A/B, submissions/binds, timing, calibrated VRAM, 8K/high-Hz/60-FPS or release acceptance is claimed because this orbit remains exact-pixel ineligible.
