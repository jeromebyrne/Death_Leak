# Post-ID67 dirty renderer read-only visual QA — 2026-10-24 00:15Z

## Verdict

**No new actionable, distinct active Forest/Waterfall visual regression is established.** The pushed `f7d04b4` changes only rotated foliage-sway repeat-X UV span, and the remaining dirty renderer diff is the pre-existing opt-in raft mip experiment. Neither supports a new default-path regression or a Windows-pixel-parity claim. This review did not build, run, capture, or change source.

## Source and prior-evidence reconciliation

- `git show f7d04b4 -- port/linux/SDLRenderer.cpp` changes only the `DrawFoliageSway` nonzero-rotation arm (`SDLRenderer.cpp:751-764`). For rotated `repeatX` with finite positive authored width no greater than texture width, it derives `uSpan=(destination.w/(2/3))/textureWidth`, then applies the existing X-flip orientation. ID67 is authored width 2000 against a 2048-pixel PNG, so its endpoint becomes 2000/2048. Zero-rotation tiling (`:741-750`), non-repeat foliage and vertex sway (`:690-739`) are unchanged. `PRIVATE_WATCHDOG_ID67_FOLIAGE_GPU_GATE_20261024T0011Z.md` already reports the physical Linux same-state A/B, 79,605 final pixels changed and product capture byte-identical to the corrected oracle. This is a tested Linux source-contract correction, not a fresh defect or matched original proof.
- At dispatch (`SDLRenderer.cpp:238-255`), foliage-sway sprites are excluded from the dirty raft branch by `RaftMipExperimentEnabled` (`:354-369`) and take `DrawFoliageSway` before ordinary rotated-repeat-X. Thus the dirty raft code cannot supersede the ID67 path, even when its environment switch is on. With `DEATHLEAK_RAFT_MIP_EXPERIMENT` absent or not exactly `on`, the new predicate is false and the ordinary path remains in use.
- `git diff -- port/linux/SDLRenderer.cpp` adds only that environment-gated `Media/objects/raft.png` private-GL mip draw (`:238-240,354-425`); the branch excludes source rectangles, repeats, foliage sway, pixel wobble, UV shift, damage gain, and non-scene draws. `PRIVATE_WATCHDOG_DIRTY_RENDERER_VISUAL_RECON_20261023T2318Z.md`, `CURRENT_RENDERER_DIRTY_VISUAL_AUDIT_2026-10-23.md`, and `RAFT_MIP_RENDERER_EXPERIMENT_2026-10-23.md` already own its conditional clamp-vs-wrap, alpha-modulation, and mip-bound questions. Those are not newly discovered active default-gameplay defects. The dirty branch has no off/on GPU acceptance or original bound-SRV evidence.
- The active ID46/ID1 mip/minification questions are already scoped in `PRIVATE_WATCHDOG_POST_ID67_NEXT_DISTINCT_VISUAL_GATE_20261024T0013Z.md` and `PRIVATE_WATCHDOG_ID1_MIP_NEXT_VISUAL_DISCRIMINATOR_20261024T0009Z.md`; this report does not duplicate or reassign them.

## Exact next gate

For this renderer diff, **keep the raft switch off in default delivery**. If the renderer owner proposes enabling it, first freeze one active Forest-1 ID11/57/58 state and capture serial same-binary switch-off/on lossless raft-local and unaffected-control crops, with draw-state/GL-error logs; then obtain a source-built or shipped original D3D10 frame at the *same* state/ordinal with raft SRV resource/view mip count, sampler/LOD and registered crop. Attribute any localized residual before a production change. A Linux off/on difference alone establishes sensitivity, not Windows parity. Independently, ID67 still needs the matched-original gate documented in its 00:11Z report; the current Linux A/B does not close that gate.

Only this unique report was written. No source, shared status/checkpoint, build, GPU/display/capture, Git index, commit or push was touched.
