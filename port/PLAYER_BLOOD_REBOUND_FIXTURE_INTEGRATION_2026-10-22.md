# Player-blood rebound fixture integration — 2026-10-22

## Scope and source contract

Updated only `port/tests/player_blood_impact_cpu_test.cpp` and `port/tests/player_blood_first_line_occlusion_cpu_test.cpp`. The original `Game/src/GameObjects/Projectile.cpp:608–711,734–773,493–505` and `port/PLAYER_BLOOD_BLOCKED_REBOUND_CPU_2026-10-22.md` establish that a nonpierceable line contact is unsplatted but rebounds, retains flight art/depth, emits a material spray when the material has particles, moves/spins while inactive, and retires after 0.25 s. The Linux nearest-center sweep and local RNG remain port approximations.

## Fixture reconciliation

- Impact fixture: the `caverock` branch now expects blocked/nonimpact state, normalized 2/6 rebound velocity, one ten-particle depth-3000 material spray, and visible flight texture. Existing water and material-less splat/inactive-envelope checks remain.
- First-line occlusion fixture: with rear water authored first, front `caverock` or unknown material still wins the geometric sweep. It checks front contact at Y=5, normalized rebound, one capped inactive movement/spin step, retained flight art/depth, no later rear splat or duplicate spray, and the 0.25 s equality/removal boundary. Known `caverock` has one spray; unknown material has no mapped particle texture and therefore no spray in this port. The inverse front-water splat check remains.
- Painter checks identify the blood texture among depth-3000 draw items so a concurrent material-spray sprite cannot masquerade as the blood flight.
- Existing `player_blood_blocked_rebound_cpu_test.cpp` was not edited; it remains the detailed one-line rebound/spray/painter/retirement fixture.

## Focused verification

Compiled and ran only these three CPU fixtures, each against `Combat.cpp`, `LevelScene.cpp`, and TinyXML with `g++ -O2 -std=c++17 -UNDEBUG -Iport/compat`:

- `player_blood_impact_cpu_test`: **PASS** — first blood line splat, material/no-material, inactive envelope, nonpierceable rebound.
- `player_blood_first_line_occlusion_cpu_test`: **PASS** — nearest blood line rebounds without rear splat; pierceable front still splats.
- `player_blood_blocked_rebound_cpu_test`: **PASS** — blocked blood rebounds, spins, emits material spray and retires at 0.25 s.

The first impact-fixture run exposed a painter-test ambiguity: a depth-3000 spray sprite could be returned in place of the flight sprite. The texture-specific check above resolved this; all three fixtures passed on rerun. `git diff --check` on the two owned fixtures passed. No Docker, GPU, broad build/integration run, Combat/renderer edit, or commit. All unrelated shared dirty-tree changes were left in place.
