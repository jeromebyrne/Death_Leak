# Forest-1 ID46 phase/seam disposition — 2026-10-23

## Decision

Of the two nominated forest defects, **ID46 is the only useful residual to assess**. The newer `FOREST_ID37_CURRENT_VISUAL_DECISION_2026-10-23.md` closes the gross lower-band suspicion as a same-frame, physical-GPU attribution to authored near-black ID37, with no measured Windows mismatch. ID46 has a real, narrower source/port **interior UV-interpolation difference**, but the tested R:98 tile boundary is **not a demonstrated visible seam**. No production patch is justified. Exact original-Windows visual parity remains unproved.

This is a bounded synthesis and current-source verification, not a fresh GPU capture. The existing R:98 accelerated lossless baseline/hide/only comparison and actual-draw GL API trace are more discriminating for this zero-signal seam than another uncontrolled gameplay screenshot. No display/GPU process was started in this lane; the exclusive physical-GPU lane is released.

## Causal source finding

Forest-1 XML ID46 (`Game/XmlFiles/levels/forest_exploration_1.xml:765-778`) is a flipped, 10000×252 repeated foliage-shadow quad with alpha 0.5 and `FoliageSway`. The original `Game/src/GameObjects/Sprite.cpp:181-191,201-234` emits **one** four-vertex strip with U spanning 0 to 10000/2048 = 4.8828125. `Game/ShaderFiles/FoliageSway.fx:14-20,41-65` deforms only its authored top corners, passes UV unchanged, and requests WRAP. Linux `port/linux/SDLRenderer.cpp:532-592` computes the deformed corners, then subdivides the whole shape by `ForEachRepeatXTile` (`port/linux/RepeatX.h:16-40`) and calls `SDL_RenderGeometry` with local 0–1 UVs and a new triangle diagonal **per tile**. On this non-parallelogram, interior barycentric UV interpolation is not identical to the original one-quad interpolation even when repeat phase is continuous. This is geometry/phase, not a missing foliage sprite or proof of a clamp seam.

At source-derived R:98 sample positions, the existing independent CPU comparison (`ID46_CPU_INTERPOLATION_DISCRIMINATOR_2026-10-23.md`) measured Linux-minus-original about 0.04–0.16 U texel and 0.66–1.37 V texels near the shared x≈871 edge and x948 interior. The V shift continues across the edge, so it is **not an abrupt seam**. The exact current `SDLRenderer.cpp` SHA-256 is `0950ec68774d376170a948328a5f25841b48b7ce534ab75a7fc537d27274ce81`, identical to the actual-draw trace report's captured renderer hash. The current tree is HEAD `5c2d335` plus unrelated dirty renderer work; this identity supports the **renderer path**, not byte-identity of the complete older executable or simulation state.

## Physical-GPU evidence already available, and its boundary

`ID46_SEAM_CURRENT_GPU_DISCRIMINATOR_2026-10-23.md` records fixed-dt R:98, physical Xorg `:0`, strict accelerated SDL OpenGL (`accelerated=1 software=0`, nouveau/NVE7), and lossless unfiltered, ID46-only, and hide-ID46 BMPs from the same scoped binary. In the correctly located x863–878/y560–709 edge crop, unfiltered minus hide-ID46 changes **0/2400 pixels**; x940–955/y590–709 interior changes **1328 pixels**. The isolated edge is nearly uniform sepia-clear `(9,8,5)`, while the PNG's outer source columns have alpha zero. This is a positive interior contribution control and a zero-signal composited seam test, not evidence that all ID46 pixels match Windows.

`ID46_DRAW_TIME_SAMPLER_GATE_2026-10-23.md` adds an actual final-frame GL instruction trace: the two adjacent visible ID46 tiles draw with the same texture 215; sampler state at draw is `GL_CLAMP_TO_EDGE` S/T with `GL_LINEAR` min/mag; their shared edge has local U=0/1 and no sampled out-of-range U. The Windows shader's WRAP request is a **state-contract difference**, but these local UVs and transparent edge columns make it noncausal for the observed zero-contribution seam. This trace was made at HEAD `c1756b2` with a different complete binary; it is not a fresh capture of HEAD `5c2d335`. Its renderer source hash matches the current file, and no claim is made that full gameplay state, light, weather, or postprocess is identical across revisions.

The earlier x≈233 prediction was corrected by including the +640 viewport center; actual draw vertices put the edge near x869.21 at top and x872.91 at bottom. The CPU texel-scale interpolation difference is not a rendered RGB comparison, and the zero-signal R:98 edge cannot establish every camera/phase/LOD state. The available Steam screenshot 14 is an unmatched compressed still, not an original-Windows lossless comparator. No human visual acceptance or shipped-binary parity is inferred.

## Next discriminating gate, if needed

Only a **nonzero-alpha, same-state** ID46 interior or repeat-boundary comparison against a lossless original-Windows frame can rank this residual for a code change. Register viewport, camera, ID46 four deformed corners, global UVs, shader time/weather multiplier, texture/SRV mip state, light/blend and postprocess boundary. If the original shows a measured crop difference with those fields matched, isolate one-quad versus tiled interpolation in a scoped control before changing production geometry or global sampler state. Another uncontrolled Linux gameplay frame or a blanket WRAP switch does not answer this gate.

## Scope and handoff

Read-only inspection of existing reports, actual current source and Git state, except this uniquely named report. No source, renderer, integration, test, status/checkpoint, ID59, XML/art, build, capture, commit, push or external delivery changed. No Death Leak game/apitrace/ffmpeg process was running at the final process check. **Physical Xorg GPU lane released.**
