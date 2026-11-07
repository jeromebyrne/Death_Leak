# Post-ID37 Forest-1 ParticleSpray source gate — 2026-10-23 20:21 UTC

## Verdict

**No distinct, unowned source-proven active Forest-1 ParticleSpray rendering mismatch is established on this current-tree read-only pass.** Do not assign a production edit or GPU/build work from it. This is not a Windows/Linux pixel-parity finding.

## Bounded source comparison

- Forest-1 authors one ParticleSpray, ID47: four looping `oak_leaf.png` particles at `kGroundFront`, with position/spread, downward direction, 64–92 size, 3.5–4 speed, 6–40-second staggered lives, 0.85–1 brightness and 0.05/0.98 fade fractions (`Game/XmlFiles/levels/forest_exploration_1.xml:780-800`). Its 50,000×2,000 dimensions are culling bounds, not a sprite quad. Windows passes these attributes through `Game/src/GameObjects/GameObjectManager.cpp:833-902` and requests deferred warm-up at `:921`; the factory initializes per-particle state at `Game/src/Particles/ParticleEmitterManager.cpp:550-715`.
- Windows builds the current draw vertices from **pre-step** XY/size, then advances motion and age, writes fade/brightness into vertex normals, recycles the looping particles, and warms after the first update (`Game/src/Particles/ParticleSpray.cpp:172-220,222-267,285-340,344-362`). It submits the particle triangles at `:128-157`; `Game/ShaderFiles/Particle.fx:52-59` multiplies sampled RGBA by brightness and sampled alpha again by fade. UV orientation is declared at `ParticleSpray.cpp:8-11`.
- Linux admits only the audited Forest ID47 in this level and loads its authored state (`port/game/LevelScene.cpp:305-370`); it conditionally ticks the spray (`:857-862`) and interleaves it at authored depth/order (`:1337-1343`). Current `port/game/ParticleSpray.h:40-80` captures pre-step XY/size, advances/fades/recycles and defers warm-up; `:83-101` submits individually projected leaf sprites with horizontal UV flip and brightness-times-fade alpha. This is a coherent implementation path, not an omitted emitter or reintroduced false 50,000×2,000 quad.

## Prior ownership and residual limits

`port/LEAF_PARTICLES_CHECKPOINT.md` owns the ID47 implementation and recorded a small Linux-only hide/enable effect (689 RGB pixels at one frame). `port/PARTICLE_ALPHA_PARITY_2026-10-22.md` already closed the source-proven brightness-in-alpha gap. `port/ORB_TRAIL_RENDER_CONTRACT_2026-10-22.md` already identified the generic pre-step-size/death-frame/runtime-warm-up questions; current `ParticleSpray.h:46,91` stores and consumes pre-step size, while Forest ID47 loops and is expressly deferred-warmed. `port/PRIVATE_FOREST_NEXT_UNOWNED_EFFECT_SOURCE_GATE_20261023T1939Z.md` previously reached the same ID47 no-new-gap disposition; this pass does not relabel its known private-RNG or SDL/D3D sampling approximations as a new defect. `port/game/ParticleSpray.h` is also pre-existing dirty/owned work. ID37, HUD Impact, ID10 opacity, orb and other already-owned spray work were excluded.

A future **distinct** gate would require a controlled four-leaf, same-render-ordinal Windows/Linux packet with original bound texture/SRV mip range and sampler plus per-particle XY, size, age, brightness and fade, then one specifically divergent draw-command field or lossless matched crop. The current source tree alone cannot infer the original global `rand()` sequence or sampled mip level, and an unmatched still or Linux-only ID47 toggle cannot establish such a defect.

Only this unique report was written. No source/XML/test/shared-checkpoint edit, build, GPU capture, game run, commit or cleanup of unrelated dirty files; exclusive ID37 GPU/build lane untouched.
