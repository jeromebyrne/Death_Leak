# Private Forest next unowned effect source gate — 2026-10-23 19:39 UTC

## Verdict

**No new, high-impact, unowned active-Forest visual-effect discrepancy is proven in this bounded source/CPU-only pass.** Do not assign a production patch from this gate. The immediately preceding Forest-1 ID6 draw-command CPU fixture passed the exact source fade/order/freeze contract (`PRIVATE_FOREST1_ID6_OCCLUSION_CPU_GATE_20261023T1936Z.md`); rerunning or re-nominating that tree would duplicate completed work. This result does not claim Windows/Linux pixel parity or rule out a future matched-frame residual.

## Examined candidate: authored oak-leaf particle effect, ID47

- Forest-1 XML `Game/XmlFiles/levels/forest_exploration_1.xml:780-801` authors one four-leaf, looping `oak_leaf.png` ParticleSpray at `kGroundFront`, with 50,000×2,000 update dimensions, downward direction, 6–40-second lifetime, brightness 0.85–1 and fade percentages 0.05/0.98. These dimensions are not a textured rectangle.
- Original loader `Game/src/GameObjects/GameObjectManager.cpp:833-902,921` passes the XML parameters into `CreateDirectedSprayLoadTime` and schedules deferred warm-up. Original `Game/src/Particles/ParticleSpray.cpp:128-158,164-184,224-267,286-340,345-362` binds the texture/alpha, submits particle triangles, builds quads from each live particle's **pre-update** position and size, advances motion/age, writes fade and brightness into vertex normals, recycles looping particles and performs the deferred warm-up.
- Linux `port/game/LevelScene.cpp:302-367,848-853` explicitly admits ID47, loads its authored parameters and particles, and updates the spray. `port/game/ParticleSpray.h:38-107,109-131` retains the pre-update draw state, lifetime fade, brightness, looping and warm-up path, then submits per-leaf sprites; `port/game/LevelScene.cpp:1338-1343` includes the spray in its painter order. This establishes source-path presence, not pixel identity.
- This is already an implemented/audited effect, not an unowned omission. `LEAF_PARTICLES_CHECKPOINT.md` documents its load, warm-up and deterministic private RNG and a Linux same-frame hide/enable delta of only 689 RGB pixels at one captured state. `PARTICLE_ALPHA_PARITY_2026-10-22.md` separately closes the brightness×fade alpha discrepancy. The current `ParticleSpray.h` is also **dirty/owned by another worker**. The private RNG and SDL-versus-D3D sampling differences remain known limitations, but no synchronized original particle state/frame shows a high-impact residual or isolates a new first divergent field. Do not edit that file or re-open those completed fixes on this evidence.

## Other active-effect exclusions / bounded next evidence

The Forest XML's six foliage-sway IDs41–46 and fog ScrollingSprites119/122 have current load/draw paths; their existing source, sampler and GPU gates are recorded in `PRIVATE_FOREST_FOLIAGE_SWAY_SOURCE_GATE_20261023T1838Z.md`, `FOREST_DYNAMIC_EFFECT_NEXT_SOURCE_GATE_2026-10-23.md`, and the ID37/ID46/raft reports. ID41–45 mip/wrap is a **source-state uncertainty**, not a proven shipped-Windows image defect. The impact-landing Player ID2 body omission in the earlier 19:27 audit is now implemented at `port/game/LevelScene.cpp:1213-1216` and its GPU/build lane is separately owned. HUD, renderer, current clip, and original-Windows matched acquisition are outside this CPU audit.

**Smallest next discriminating fixture if fresh evidence arises:** preserve one same-render-ordinal original-Windows lossless active Forest-1 crop and bound particle/foliage state (ID, texture/SRV mip count, sampler, particle XY/age/brightness or foliage time/weather/vertices) alongside the corresponding Linux draw-command state. For an ID47 claim, first assert a specific divergent command field under a controlled four-particle state; for ID41–45 sampling, first prove the original bound SRV has and samples higher mips. Only then propose a field-scoped owner fix and matched crop gate. An unmatched Steam image or existing Linux-only hide/enable delta cannot establish this.

## Scope and release

Inspected current HEAD `e9e6be4` and pre-existing dirty ownership; no build, test execution, GPU, Docker, Windows runtime, source/test edit, shared checkpoint/status edit, Git index, commit or push. Only this uniquely named report was written. **CPU/read-only lane explicitly released.**
