# Combat mixed-diff integration — 2026-10-22

## Decision

**Do not stage, commit, or push this shared-file diff yet.** The blood-flight/impact/rebound work and the player shot aim/launch-state work are each source-backed in their bounded contracts, and their focused CPU gates pass together. But a commit restricted to this worker's ownership (`port/game/Combat.cpp`, `.h`, and the three `player_blood_*` CPU fixtures) would include shot behavior while omitting the dirty live caller in `port/linux/main.cpp` and the untracked `port/tests/player_shot_launch_state_test.cpp`, which are owned outside this task. The shared `SpawnKnifeLineSpray` direction change also couples both slices. A whole-file Combat commit is therefore not an ownership-safe, independently reviewable blood commit. All unrelated dirty files remain unstaged and unchanged.

## Reviewed source and shared hunk map

- Blood: `Game/src/GameObjects/Character.cpp` damage/blood spawn and `Game/src/GameObjects/Projectile.cpp:413–588,608–773` support the flight, line impact, pierceable splat, nonpierceable rebound, inactive envelopes, material spray, and launch-direction ordering. In `Combat.cpp` this comprises `BloodSegmentContact`, blood art/material loading, `SpawnPlayerBloodFlight`, the blood phase at the top of `Update`, accepted player-hit spawning, and painter items. In `Combat.h` it comprises the three blood probes, clear/count, blood state/RNG/material fields, art IDs, and spawn declaration. Nearest-center sweep, deterministic local RNG, and the active orphan bound remain documented Linux approximations.
- Shot: `Game/src/InputManager.cpp` right-stick aim/fire and `Game/src/GameObjects/Player.cpp:424–455` support the bounded normalized aim, launch Y branch order, and launch-direction spray. In `Combat.cpp` this comprises the `SpawnKnifeLineSpray` direction change, `Update` signature, and player-knife spawn block. In `Combat.h` it comprises `Shot::launchDirection*`, `PlayerShotLaunchState`, and the extended `Update` declaration/defaults. This is not a claim of full Windows shot-origin/input parity. The live caller and shot fixture are outside this ownership scope.
- **Cross-slice dependency:** blood uses `SpawnKnifeLineSpray`; removing the shot helper's launch-direction change while retaining blood code would silently change blood spray behavior. The two features are not independent by hunk selection alone.

## Correction made during review

On a nonpierceable blood-line contact, source `Projectile::HandleSolidLineStripCollision` emits the spray using the original zero `m_direction`, then calls `ReboundOffSolidLine` to assign a new direction. The pending Combat code had set a synthetic downward launch direction for that spray. Removed that assignment in `Combat.cpp` and corrected the blocked-rebound fixture's assertion to expect zero launch direction plus the helper's spread. This changes only owned files; the source approximation and rebound motion remain unchanged.

## Verification after correction

Built each fixture with `g++ -O2 -std=c++17 -UNDEBUG -Iport/compat` against `port/game/Combat.cpp`, `port/game/LevelScene.cpp`, and the four `Game/tinyxml` translation units; all five ran successfully:

- `player_blood_impact_cpu_test`: PASS.
- `player_blood_first_line_occlusion_cpu_test`: PASS.
- `player_blood_blocked_rebound_cpu_test`: PASS.
- `player_hit_blood_visual_test`: PASS (read-only regression fixture).
- `player_shot_launch_state_test`: PASS (read-only shot fixture).

`git diff --check -- port/game/Combat.cpp port/game/Combat.h` passed. Each of the three untracked blood fixtures passed `git diff --no-index --check /dev/null <fixture>` (exit 1 denotes content difference, with no whitespace report). No GPU, Docker, renderer edit, or broad build was used.

## Concrete separation/integration patch plan

1. Have the shot owner integrate/review `port/linux/main.cpp` and `port/tests/player_shot_launch_state_test.cpp` together with the shot-specific Combat hunks. Re-run the shot fixture and a main translation-unit syntax check in that owner's scope. Do not infer that the current dirty main file is wholly shot-owned.
2. For a blood-only commit before shot integration, extract only the blood hunks listed above using an explicit index patch (`git apply --cached` or interactive hunk staging), then make blood spray direction independent of the shot helper: add a focused direction parameter or blood-specific helper path that passes zero launch direction for both splat and rebound. Keep shot helper/signature/knife changes and all main/renderer files unstaged. Validate the staged tree, not just the worktree, with the three blood fixtures and active-flight regression.
3. Alternatively, after the shot owner explicitly takes ownership of the live caller and shot fixture, jointly stage the reviewed Combat, blood fixtures, shot fixture, and **only the corresponding main caller hunk** as one integrated software change; run the five CPU gates and main syntax check on that exact staged snapshot. Do not stage whole dirty files with unrelated hunks.
4. In either route, inspect `git diff --cached --stat`, `git diff --cached --check`, and the full staged patch before committing. Push only after the resulting commit is ownership-safe and its relevant focused gates pass. This task did not stage, commit, or push.
