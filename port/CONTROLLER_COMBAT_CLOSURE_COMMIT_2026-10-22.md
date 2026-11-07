# Controller + Combat dependency closure — committed 2026-10-22

## Result

Committed and pushed the exact 13-path non-GPU controller/Combat dependency closure from `CONTROLLER_DEPENDENCY_INTEGRATION_2026-10-22.md` and `COMBAT_SHOT_BLOOD_INTEGRATED_2026-10-22.md`.

- Branch: `feature/linux-renderer-foundation`
- Base: `2c61efb16f28e186fcf28b82002b27acc6e42d6f`
- Commit: `8f8d8179708e056bf02be1a7c7fc27440f337e69` (`Port controller input and player combat shot/blood closure`)
- Remote `origin/feature/linux-renderer-foundation`: independently confirmed by `git ls-remote` at the same commit after push.
- Commit boundary: 13 paths, 835 insertions, 47 deletions. No status, integration script, GPU, Docker, or capture files included.

## Frozen candidate and gates

Verified all 13 worktree hashes matched the blobs recorded in the controller integration gate, with HEAD still at the cited base. Created isolated `GIT_INDEX_FILE=/tmp/deathleak-closure-final.c67BuR/index` from HEAD, added only those 13 paths, and materialized its tree at `/tmp/deathleak-closure-final.c67BuR/tree`. `git diff --cached --check` passed there. The real index was not used for candidate staging.

On that materialized snapshot, `g++ -std=c++17 -fsyntax-only -Iport/compat` passed for `port/linux/main.cpp`, `port/game/PlayerMotion.cpp`, `port/game/Combat.cpp`, and `port/game/LevelScene.cpp`. Five standalone `g++ -O2 -std=c++17 -UNDEBUG -Iport/compat` CPU fixtures, each linked with snapshot Combat/LevelScene and the four tracked tinyxml translation units, compiled and returned 0:

- `player_blood_impact_cpu_test`
- `player_blood_first_line_occlusion_cpu_test`
- `player_blood_blocked_rebound_cpu_test`
- `player_hit_blood_visual_test`
- `player_shot_launch_state_test`

The six-line `SDLRenderer.cpp` connected-axis override is included in the committed closure. Its SDL translation unit was not syntax-compiled because this host's `pkg-config` lacks the combined `sdl2`/`glew` prerequisites; this is not a GPU/runtime proof. No full build, link, Xorg, capture, or GPU test was claimed.

## Shared-worktree disposition

The isolated-index commit temporarily left the real index relative to the new HEAD. Reset only the 13 committed paths in that index; verified it has no staged entries. The only remaining tracked dirty paths are the preexisting `CHECKPOINT_STATUS.md`, `port/COMPOSITION_DIAGNOSTIC_NOTES.md`, and `port/tests/integration.sh`. Preexisting untracked files remain untouched, including `PORT_WORK_STATUS.md`. This handoff is the sole new file written by this lane and is intentionally not part of the exact 13-path software commit.
