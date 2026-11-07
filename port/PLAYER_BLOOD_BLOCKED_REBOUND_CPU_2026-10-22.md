# Blocked player-blood rebound CPU handoff — 2026-10-22

## Original source contract

- `Game/src/GameObjects/Projectile.cpp:608–711`: a solid-line contact stops/marks the projectile inactive, emits a 10-particle directed material spray when the strip has a material, then either swaps to impact art for pierceable material or calls `ReboundOffSolidLine` for nonpierceable material. Material-less contact uses impact art, without a material spray.
- `Projectile.cpp:734–773`: rebound takes the collided line normal, adds `rand()%20` Y and signed `rand()%20` X, normalizes, assigns X/Y velocity multipliers 2/6, retains flight art, sets 0.25 s inactive life, and spin increment 0.09.
- `Projectile.cpp:493–505`: while inactive, the projectile still moves by velocity times capped 60 Hz `percentDelta`, rotates +0.09 per update, and moves Y by another -1.15 per `percentDelta`.
- `Game/src/GameObjects/SolidLineStrip.cpp:309–319`: line normal is the perpendicular of the normalized *authored point order*. `LevelScene` preserves that endpoint order in `GroundSegment`; no new scene field is necessary. The Linux center sweep and local deterministic RNG remain approximations, not a Windows object-order/global-rand claim.

## Scoped implementation

`Combat.cpp/.h` now recovers the authored normal for a blocked player-blood contact, applies deterministic local rebound offsets/velocity, advances and spins the inactive flight, and emits the known material's directed spray on that branch. It retains the existing unsplatted flight art/depth and 0.25 s retirement. A zero-length segment yields zero rebound velocity rather than division by zero. No renderer, scene, main, GPU, Xorg, Docker, or broad integration work was touched.

Added `port/tests/player_blood_blocked_rebound_cpu_test.cpp`. Its authored `caverock` contact checks no impact, normalized 2/6 rebound velocity, material spray with 10 particles at player depth, reverse-downward spray direction, exact one-tick inactive movement and spin, painter flight texture/rotation, and 0.25 s retirement.

## Verification and integration note

- Focused compile/run: **PASS** — `PASS: blocked blood rebounds, spins, emits material spray and retires at 0.25s`.
- `git diff --check -- port/game/Combat.cpp port/game/Combat.h`: **PASS**.
- Preexisting `player_blood_impact_cpu_test.cpp`: **FAIL at line 99** because it asserts no spray for the nonpierceable branch; the source-backed implementation intentionally emits one.
- Preexisting `player_blood_first_line_occlusion_cpu_test.cpp`: **FAIL at line 78** because it asserts blocked blood stays at contact Y with zero velocity; the source-backed implementation intentionally rebounds and moves. These two fixtures are outside this worker's file ownership and require expectation updates by their owners before integration.
- No commit/push: `Combat.cpp/.h` already carried uncommitted changes from other workers and the existing fixtures require coordinated update. This worker preserved unrelated dirty changes.

Focused command (repository root):

```sh
g++ -O2 -std=c++17 -UNDEBUG -Iport/compat port/tests/player_blood_blocked_rebound_cpu_test.cpp port/game/Combat.cpp port/game/LevelScene.cpp Game/tinyxml/tinyxml.cpp Game/tinyxml/tinyxmlerror.cpp Game/tinyxml/tinyxmlparser.cpp Game/tinyxml/tinystr.cpp -o /tmp/player_blood_blocked_rebound_cpu_test && /tmp/player_blood_blocked_rebound_cpu_test
```
