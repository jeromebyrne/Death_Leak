# Ninja common material spray implementation — 2026-10-22

Implemented the bounded source-backed `demon1` lethal material spray described in `NINJA_MATERIAL_SPRAY_IMPLEMENTATION_AUDIT_2026-10-22.md`.

- On a lethal `kNinja` hit, `Combat::DamageEnemy` now emits a separate, nonlooping 20-particle `Game/Media/bloodparticle.png` spray before the existing NPC debris and explosion lines. Other enemy types and nonlethal hits do not emit it; the dead-enemy guard prevents duplicates.
- The emitter's world origin is post-projectile-kickback `(enemy.x, enemy.y - 231.714279/2 + 30)`, derived from source `GameObject::Bottom()` using the ninja's visual dimensions. Depth is `kPlayer` (3000). The renderer applies the normal 2/3 projection once.
- Runtime-factory particle samples use an emitter-private deterministic seed: direction `(0.2+r, 0.8-r)` with `r=0..0.29` in 0.01 increments; signed integer spawn offsets up to 50 X/100 Y; size 11..40; speed 7.01..12; brightness .76..1; gravity 1.4; indexed lifetimes 1.5..0.835 seconds. Fade-in .15, fade-out .8, lifetime scale target 8. The spray is pre-warmed to bypass authored XML's deferred `Warm()` behavior.
- `Combat::Update` advances and drains the emitter; `ClearLevel` clears it; the unified painter receives a depth-3000 draw callback. Existing `DeathSprayCount()` continues to mean the five explosion-line effects; separate material accessors expose the new emitter for focused verification.

## Verification

Built `port/tests/ninja_death_effect_test.cpp` in independent `/tmp/ninja-material-spray-build-20261022` with `g++` against `Combat.cpp`, `LevelScene.cpp`, and tinyxml. The focused CPU test passed. It checks nonlethal absence, one 20-particle texture-backed lethal spray, post-kickback visual-bottom origin, source ranges and lifetime sequence, nonduplication, draw participation, and natural drain while preserving 10 debris/5 lines. `git diff --check` passed for the scoped files. No Docker build or GPU capture was run by this lane.

GPU validation of the arena lethal event is **pending** with the separate build/GPU owner. This CPU-backed slice does not establish release or full visual parity; Windows global `rand()` alignment and Windows backbuffer-dependent particle scaling remain unresolved parity questions.
