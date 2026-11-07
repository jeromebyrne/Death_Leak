# Directed blood impact position repair — 2026-10-23

## Scope and source contract

`Game/src/GameObjects/Projectile.cpp:410-491,609-638` advances the projectile to its full next center before a solid-line collision stops velocity; collision does not reposition it to the ray contact. The port's directed blood ray extends from the old center to `next center + launch direction * 32`, and chooses the nearest line hit. Previously it reused that ray fraction to interpolate the *shorter* old-to-next-center movement, placing the impact/rebound sprite short of the source center.

## Repair

`port/game/Combat.cpp` now retains `nextX,nextY` for directed line collisions, before the existing splat jitter or blocked rebound. Nearest-hit selection and ray-end material-spray origin are unchanged. The zero-direction ordinary branch keeps its prior center-sweep placement, as required by the existing ordinary-hit regression. No `Combat.h` change was needed.

New `port/tests/blood_directed_impact_position_cpu_test.cpp` covers a water line at x=-20 that intersects only the extended directed ray (not the advanced center); checks impact at the full advanced center (-5.6,+4.4 before jitter), deterministic post-jitter Y, and spray at the ray end (-56.8,+49.2). Its second case guards ordinary zero-direction contact and ordinary spray origin. The fixture uses a private temp XML and no GPU.

## Verification

From repository root, compiled with `g++ -O2 -std=c++17 -UNDEBUG -Iport/compat`, linking each fixture with `port/game/Combat.cpp`, `port/game/LevelScene.cpp`, and the four `Game/tinyxml/tinyxml*.cpp` implementation units, output to separate private `/tmp/blood-*-XXXXXX` directories:

- New `blood_directed_impact_position_cpu_test.cpp`: **PASS** — `directed ray-only impact center/spray and ordinary zero-direction guard`.
- Existing, unmodified `player_blood_impact_cpu_test.cpp`: **PASS** — `first blood line splat, material/no-material, inactive envelope, nonpierceable rebound`.
- `git diff --check -- port/game/Combat.cpp port/game/Combat.h`: **PASS**. Scoped code diff is only the directed collision position conditional and comment.

No shared `port/build`, physical GPU `:0`, renderer, scene code, integration harness, coordinator/checkpoint, commit, or push was used. Other dirty files were untouched. This is source/CPU parity verification, not a visual acceptance claim.

**File ownership released:** `port/game/Combat.cpp`, `port/game/Combat.h` (read-only), `port/tests/blood_directed_impact_position_cpu_test.cpp`, and this report are free for coordinator integration; no further edits are pending from this owner.
