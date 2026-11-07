# Fresh-save blood burst horizontal UV: private CPU implementation (2026-10-23)

## Change

- `port/game/ParticleSpray.h`: set `leaf.flipX = true` on each live ParticleSpray sprite before submission. Original `Game/src/Particles/ParticleSpray.cpp` binds U=1 to world-left and U=0 to world-right in the active vertex path; the alternate per-particle flip code is commented out. The Linux sprite's default was unflipped. `flipY` remains false. No emission, position, size, tint, alpha, or depth logic changed.
- `port/tests/fresh_save_blood_burst_flip_private_cpu_test.cpp`: private renderer-spy fixture checks a visible particle submits horizontal flip only, keeps texture/dimensions/alpha, and preserves dead/offscreen filtering.

## CPU-only verification

- `g++ -std=c++17 -Wall -Wextra -Werror -Iport port/tests/fresh_save_blood_burst_flip_private_cpu_test.cpp -o /tmp/deathleak-blood-burst-flip-cpu-$$ && /tmp/deathleak-blood-burst-flip-cpu-$$` — PASS.
- `git diff --check -- port/game/ParticleSpray.h` — PASS.
- No shared binary build or GPU/Xorg/Docker operation performed. CPU fixture proves the submitted sprite state, not final pixels or Windows visual acceptance.

## Scope and handoff

Only `port/game/ParticleSpray.h`, this new private fixture, and this new private report were changed in this lane. Existing unrelated dirty files were preserved. No commit or push. After the exclusive X-prompt GPU owner releases the physical GPU/build lane, integration can capture a fixed-frame fresh-save held-pull burst and compare an isolated asymmetric particle against the source U mapping. **Horizontal-UV CPU lane released.**
