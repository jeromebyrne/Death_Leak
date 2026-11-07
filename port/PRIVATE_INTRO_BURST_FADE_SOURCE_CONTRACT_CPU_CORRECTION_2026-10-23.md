# Intro sword-pull burst fade: source-contract correction (private, 2026-10-23)

## Verdict and evidence

The original 20- and 50-particle sword-pull `CreateDirectedSpray` calls in `Game/src/GameObjects/Player.cpp:1035-1083` each end `scaleTo, 0.1f, 0.8f, 5.0f, 1.0f`. The original declaration and implementation in `Game/src/Particles/ParticleEmitterManager.h:76-82` and `.cpp:390-397` map those positions to `scaleTo, spawnSpreadX, spawnSpreadY, fadeInPercentTime, fadeOutPercentTime`. The implementation uses the `0.1/0.8` values for spawn-offset calculations (`.cpp:442-475`) and passes the final `5/1` values into the fade setters (`.cpp:540-541`). Thus the prior private implementation report reversed the source argument mapping.

The source alpha calculation in `Game/src/Particles/ParticleSpray.cpp:233-258` uses fade-out only if `percentTimeLeft < 1 - fadeOut`; with fadeOut=1 that branch does not fire for a live particle. Its fade-in branch gives alpha `elapsedFraction / 5`. The port's analogous update is `port/game/ParticleSpray.h:40-60`; the directed intro assignment in `port/game/LevelScene.cpp:1094` is now `spray.fadeIn = 5.f; spray.fadeOut = 1.f`. This restores its pre-existing HEAD value: the competing `.1/.8` edit was an unstaged one-line change at task start, and there is now no `LevelScene.cpp` diff against HEAD. The separate `.1/.8` spawn offsets and horizontal texture flip were not changed.

## Focused CPU check

Updated only `port/tests/fresh_save_blood_burst_fade_private_cpu_test.cpp` to extract the actual intro emitter fade assignment, assert 5/1, and exercise `ParticleSpray::Update` and `Draw` through a recording renderer. At elapsed fractions 0.20 and 0.90, alpha is 0.04 and 0.18, with rounded 8-bit alpha 10 and 46; horizontal flip remains enabled. `g++ -std=c++17 -Wall -Wextra -Werror -Iport port/tests/fresh_save_blood_burst_fade_private_cpu_test.cpp -o <temporary binary>` followed by fixture execution with `port/game/LevelScene.cpp`: PASS. `git diff --check HEAD -- port/game/LevelScene.cpp`: PASS. `git status --short -- port/game/LevelScene.cpp` is clean. This is source/CPU evidence, not Windows/GPU image-fidelity acceptance.

## Scope and release

Only the intro fade assignment, the private focused fixture, and this private report were touched. No Docker, shared build, GPU, Xorg, shared status/checkpoint, commit, or push. **Exclusive intro burst fade lane released.**
