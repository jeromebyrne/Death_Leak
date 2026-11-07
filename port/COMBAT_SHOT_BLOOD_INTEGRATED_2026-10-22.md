# Combat shot/blood scoped integration gate — 2026-10-22

## Result

**Not committed or pushed.** The combined shot/blood Combat worktree passed all five focused CPU fixtures and the live `main.cpp` syntax check. A scoped, isolated candidate index containing only `Combat.cpp`, `Combat.h`, the five fixtures, and the shot caller hunk in `main.cpp` is **not independently buildable** against HEAD. The caller's right-stick path depends on unowned, still-dirty `port/render/Renderer.h` (`ControllerSticks` and `Renderer::ControllerAxes`) and `port/game/LevelScene.h` (`RightStickCameraTarget::WorldRightY`). Staging these dependencies would exceed this worker's file ownership; omitting them would produce a broken commit. No real index was changed.

## Reviewed integration

Read `COMBAT_MIXED_DIFF_INTEGRATION_2026-10-22.md` first. The shared `SpawnKnifeLineSpray` uses `Shot::launchDirection*`; default-zero blood emitters preserve original pre-rebound direction on both splat and blocked contacts, while aimed knives pass their launch direction. `Game/src/GameObjects/Projectile.cpp:608–711,734–773` supplies the blood impact/rebound/spray ordering; `Game/src/GameObjects/Player.cpp:424–455` and `Game/src/InputManager.cpp` supply shot launch/aim ordering. The current code maintains the documented Linux-only nearest-center sweep, local RNG, and active-orphan bound approximations.

## Evidence

- Built with `g++ -O2 -std=c++17 -UNDEBUG -Iport/compat` against each fixture, `Combat.cpp`, `LevelScene.cpp`, and the four `Game/tinyxml` translation units. Each executable returned 0: `player_blood_impact_cpu_test`, `player_blood_first_line_occlusion_cpu_test`, `player_blood_blocked_rebound_cpu_test`, `player_hit_blood_visual_test`, `player_shot_launch_state_test`.
- `g++ -std=c++17 -fsyntax-only -Iport/compat port/linux/main.cpp` returned 0 on the **live dirty worktree**. This is not a staged-build claim.
- Built an isolated temporary Git index at `/tmp/combat-index-dd56hrjr/index` from HEAD, adding byte-identical current `Combat.cpp/.h`, all five fixtures, and an explicitly extracted main caller patch only. Its stat was 8 paths, 696 insertions and 11 deletions; `git diff --cached --check` with that index passed. The main patch contains the controller read, deadzone/aim/fire, launch-state calculation, and extended Combat call; it excludes camera-follow, strafe, and other main edits. Full `main.cpp` staged patch was inspected.
- A syntax compile of that isolated candidate `main.cpp`, with HEAD `Renderer.h` and HEAD `LevelScene.h`, failed because `ControllerSticks` is undefined, `Renderer::ControllerAxes` is absent, and `RightStickCameraTarget` is absent. This is the decisive staged-snapshot build gate failure. `LevelScene.h` at HEAD also already contains a `PlayerStrafeVisual` reference to the unstaged renderer `ControllerSticks`, so broader HEAD integration is independently inconsistent.

## Ownership disposition

No source edit was needed. The Combat and five fixture bytes remain in the dirty worktree, unstaged. The real index, all unrelated dirty files, `CHECKPOINT_STATUS.md`, and `PORT_WORK_STATUS.md` were left untouched. No GPU, Docker, renderer edit/capture, commit, or push was performed. Integration needs the owner of the renderer/controller and right-stick camera changes to land those dependencies (or an explicitly authorized cross-owner commit); then reconstruct the scoped index, rerun all five fixtures and main syntax/build against that exact snapshot, inspect its full patch, and commit/push only if green.
