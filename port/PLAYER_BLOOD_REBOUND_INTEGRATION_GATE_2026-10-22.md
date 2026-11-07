# Player-blood rebound Combat integration gate — 2026-10-22

## Result

**CPU integration passes; commit gate blocked by mixed shared-file diffs.** Reviewed the blood-flight/rebound changes in `port/game/Combat.cpp` and `.h`, all three `player_blood_*` CPU fixtures, the two prior handoffs, and the original `Game/src/GameObjects/Projectile.cpp` collision/rebound/inactive branches. No remaining source-backed defect was verified within this bounded blood slice. The Linux nearest-center sweep, local deterministic RNG, and orphan bound remain documented port approximations.

## Focused verification

Built each fixture with `g++ -O2 -std=c++17 -UNDEBUG -Iport/compat` against `Combat.cpp`, `LevelScene.cpp`, and TinyXML, then ran it:

- `player_blood_impact_cpu_test`: PASS — first blood line splat, material/no-material, inactive envelope, nonpierceable rebound.
- `player_blood_first_line_occlusion_cpu_test`: PASS — nearest blood line rebounds without rear splat; pierceable front still splats.
- `player_blood_blocked_rebound_cpu_test`: PASS — blocked blood rebounds, spins, emits material spray and retires at 0.25 s.
- `git diff --check --` the two Combat files and three fixtures: PASS.

## Commit/push gate and next action

No files staged, no commit, no push. The working-tree diff in `Combat.cpp/.h` interleaves the blood work with unrelated player-knife aim/launch-state changes (including the `Combat::Update` signature, knife-spawn logic, and launch-direction spray). A whole-file stage/commit would include those unrelated hunks, contrary to this gate's scope. Other dirty repository files were preserved untouched.

Next: coordinate with the Combat/shot-launch owner to separate or explicitly integrate those shared-file changes, then stage a reviewed blood-only patch plus the three fixtures and rerun this focused gate before committing/pushing. Do not whole-file stage the current Combat diff as a blood-only commit.
