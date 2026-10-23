# Ninja material spray first-frame alpha fix (2026-10-22)

## Source-grounded finding

The Windows runtime factory creates the 20 particles before adding its `ParticleSpray` (`Game/src/Particles/ParticleEmitterManager.cpp:426-525`). The original `ParticleSpray::Update` computes fade-in alpha from elapsed lifetime (`Game/src/Particles/ParticleSpray.cpp:150-260`). The port correctly sets `.15` fade-in, but `Combat::Update` updates existing material sprays before collision/knife damage; an Update-spawned lethal ninja spray therefore misses that pass. Previously each new port particle retained `Particle::alpha=1`, causing its first rendered frame to be fully opaque, apart from brightness. The exact Windows creation-to-first-draw scheduling has not been established, so this fixes a concrete port artifact rather than claiming exact Windows first-frame equivalence.

## Bounded change

`Combat::SpawnNinjaMaterialSpray` now initializes each newly emitted particle alpha to zero (fade-in time zero). Subsequent `ParticleSpray::Update` retains the existing source-shaped `.15` fade-in calculation; no global emitter behavior or debris/explosion line timing changed. No `Combat.h` change was needed.

The focused `ninja_death_effect_test.cpp` now kills a ninja via a knife within `Combat::Update`, captures the **rendered** blood sprites immediately, and asserts that all 20 have zero tint alpha. After a 1/60-second emitter update it asserts all 20 have positive, sub-opaque rendered alpha. This covers the ordering missed by the earlier direct-`DamageEnemy` test.

## Verification and remaining gate

- Focused CPU test compiled in `/tmp/ninja-spray-alpha-fix-20261022` with `g++ -std=c++17 -O0 -g -Wall -Wextra -Wpedantic` against Combat, LevelScene, and tinyxml; passed (`PASS: ninja kill effects and Update-spawned first-frame fade alpha`). Only pre-existing tinyxml compiler warnings appeared.
- `git diff --check` passed for the scoped patch.
- No Docker build or physical-GPU capture was run; the separate post-modal worker owns that resource. A physical-GPU arena kill capture/review remains required for visual sign-off, including painter integration, first-visible-frame appearance, and the unresolved source-backbuffer scale question noted by source QA.
