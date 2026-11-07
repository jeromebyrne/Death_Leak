# Private next unowned Forest source gate — 2026-10-23 20:04Z

## Verdict

**Negative finding:** this read-only check found no still-unowned, source-proven, high-impact visual discrepancy in the first 10 seconds of active Forest-1. No bounded production implementation is justified by the current evidence. This is not an assertion of Windows/Linux pixel parity: the available 300-frame active-play clip is Linux-only, and no same-render-ordinal original-Windows lossless image plus bound draw state exists locally.

## Current source/render contract checked

- Original Forest-1 authors Player ID2 and overlapping middle/near-foreground trees/strips in `Game/XmlFiles/levels/forest_exploration_1.xml:33-46,176-190,277-366,612-665`; `Game/src/GameObjects/GameObjectManager.cpp:302-330,1021-1025` draws visible objects in depth-sorted order. The ordinary light-texture pixel shader samples texture/light color and multiplies sampled alpha by object alpha once (`Game/ShaderFiles/BasicLightTexture.fx:53-59`). Thus a Linux-only dark or pale region is not by itself evidence of omitted scenery or incorrect alpha.
- Current port stably depth-sorts scene objects (`port/game/LevelScene.cpp:625`), interleaves dynamic objects/particles with the painter list (`:1335-1350`), computes destination/parallax/UV and submits texture, tint, opacity, flips, effect clocks and rotation (`:1354-1449`). It draws the player's valid arm before body (`:1440-1449`). These paths establish present source plumbing, not exact original texture filtering, blend output or animation phase.
- Existing controlled reports already own the visible high-area questions: ID10 player attenuation and ID37 lower band (`PRIVATE_NEXT_UNOWNED_FOREST_VISUAL_SOURCE_GATE_20261023T1954Z.md`; `PRIVATE_FOREST_9S_OCCLUSION_GPU_DISCRIMINATOR_20261023T1944Z.md`), ID6 fade (`PRIVATE_FOREST1_ID6_OCCLUSION_CPU_GATE_20261023T1936Z.md`), middle-band IDs17/19/21/22 (`FOREST_MIDDLE_BAND_CURRENT_FRAME_GPU_CAUSAL_2026-10-23.md`), particle ID47 and foliage/fog effects (`PRIVATE_FOREST_PARTICLE_EFFECT_NEXT_GATE_20261023T1959Z.md`), player arm/body timing (`PRIVATE_FOREST_PLAYER_ANIMATION_VISUAL_SOURCE_GATE_20261023T1944Z.md`), and scene draw-state (`PRIVATE_NEXT_UNOWNED_FOREST_DRAWSTATE_GATE_20261023T2002Z.md`). The impact-landing body-selection omission from the 19:27 non-HUD audit has since been implemented; it is not a new nomination. HUD Impact-face, clear color, and original-Windows acquisition are explicitly outside this lane.

## Concrete prerequisite, not a patch

The first useful new discriminator is an **atomic, same-render-ordinal original-Windows Forest-1 packet**: qualified runnable game EXE and D3D10 host, lossless 1280×720 post-HUD/pre-Present frame, camera/player/effect history, and IDs plus texture/SRV mip range, sampler, transform/UV, alpha, blend and draw ordinal. `PRIVATE_WINDOWS_FOREST_RUNTIME_ACQUISITION_GATE_20261023T1955Z.md` documents the immediate blocker: no accessible game EXE or callable Windows/Wine/Proton runtime in this lane. With such a packet, register a Linux frame at the measured state, use a production draw-command trace to identify the first divergent field, then assign only that field and its crop to an exclusive implementation/GPU owner. Without it, the existing Linux-local deltas and unmatched Steam JPEG cannot prove a further original-parity defect. **Minimal current implementation: none.**

## Scope and release

Only this uniquely named report was written. Existing dirty source/reports, Git index, `CHECKPOINT_STATUS.md`, and `PORT_WORK_STATUS.md` were preserved. No code edit, test, build, Docker, GPU/display, capture, game runtime, commit or push. **Read-only Forest source lane explicitly released.**
