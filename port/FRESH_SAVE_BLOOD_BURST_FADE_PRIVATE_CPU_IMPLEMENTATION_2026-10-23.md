# Fresh-save sword-pull blood-burst fade: private CPU implementation (2026-10-23)

## Source contract and patch

Original `Game/src/GameObjects/Player.cpp` sword-pull calls use `fadeInPercentTime=0.1f` and `fadeOutPercentTime=0.8f` for both the 20-particle and 50-particle directed sprays. The argument order is confirmed by `Game/src/Particles/ParticleEmitterManager.cpp`'s `CreateDirectedSpray` signature and setter calls. The calls' `5.0f, 1.0f` values are spawn spreads, not fades. `port/game/LevelScene.cpp` had assigned those spread values to `spray.fadeIn`/`spray.fadeOut` in the common two-recipe loop. Changed that single assignment to `.1f`/`.8f`; particle counts, spread generation, art, UV orientation, and generic rendering remain untouched.

## Focused CPU verification

`port/tests/fresh_save_blood_burst_fade_private_cpu_test.cpp` extracts the fade assignment from the actual intro emitter source scope and checks that it is applied by the two-recipe loop. It runs the current `ParticleSpray::Update`/`Draw` through a recording renderer with a 0.75-second, brightness-1 particle. At 20% lifetime alpha/tint is 1/255; at 90% it is 0.5/128. Horizontal U flip remains true.

- `g++ -std=c++17 -Wall -Wextra -Werror -Iport port/tests/fresh_save_blood_burst_fade_private_cpu_test.cpp -o /tmp/deathleak-blood-burst-fade-cpu-$$ && /tmp/deathleak-blood-burst-fade-cpu-$$ port/game/LevelScene.cpp` — PASS.
- `git diff --check -- port/game/LevelScene.cpp` — PASS.
- Reviewed `git diff -- port/game/LevelScene.cpp`: exactly one assignment line changed.

This is source/CPU proof, not a GPU pixel or Windows screenshot acceptance. Concurrent GPU work may remain on its prior frozen binary.

## Ownership release

This lane touched only `port/game/LevelScene.cpp`'s fade assignment, the private focused fixture, and this report. Shared status/checkpoint, build, Docker, Xorg/GPU, `main.cpp`, `SDLRenderer`, and unrelated dirty files were not touched. No commit or push. **Fresh-save blood-burst fade CPU lane released.**
