# Controller dependency integration gate — 2026-10-22

## Verdict

The Combat/shot/blood caller is **syntax-buildable** only as part of a wider controller closure. It is **not safely committable by this lane alone**: `port/linux/SDLRenderer.cpp` supplies the live `ControllerAxes()` override but is outside this lane's source ownership, while `Combat.cpp/.h` and five fixtures belong to the Combat owner. Omitting the SDL override still compiles because `Renderer::ControllerAxes()` defaults to disconnected axes, but silently disables the controller behavior. No commit, push, real-index staging, or source edit was made.

## Exact dependency closure from HEAD `2c61efb`

- `port/render/Renderer.h` adds `ControllerSticks` and the virtual `ControllerAxes()` seam. `port/game/LevelScene.h` consumes the DTO for strafe and next-frame camera target, including SDL-Y to world-Y conversion used by the shot caller.
- `port/game/PlayerMotion.h/.cpp` expose impact-recovery state consumed by `main.cpp`'s camera X-hold gate. The main caller also uses `LevelScene.h`'s `RightStickCameraTarget`, `FollowCameraWithTarget`, and `ResolvePlayerStrafeVisual`.
- `port/linux/main.cpp` contains inseparable current dirty main-loop changes: controller snapshot, strafe/body visual call, camera follow/reset/update, and the scoped Combat shot launch/aim/fire call. `port/game/Combat.h/.cpp` must be included for the extended Combat API and shot/blood implementation. The five new blood/shot focused fixtures accompany it.
- **Runtime-only but essential:** `port/linux/SDLRenderer.cpp` implements the actual connected raw-axis read. This is a six-line dirty change owned elsewhere. Its default-free virtual fallback otherwise returns disconnected values and defeats the feature without a compile error.
- `LevelScene.cpp` and existing SDL renderer header are unchanged at HEAD for this closure. `port/tests/integration.sh` is dirty but not required by the syntax/runtime API closure and was excluded.

## Isolated staged snapshot and verification

An isolated `GIT_INDEX_FILE=/tmp/deathleak-controller-closure.UyAZgl/index` was initialized via `git read-tree HEAD`, then populated with exactly the current bytes of the 13 paths below. `git checkout-index --all` materialized that index under `/tmp/deathleak-controller-closure.UyAZgl/tree`. The real Git index remains empty. Candidate diff: **13 paths, 835 insertions, 47 deletions**; `git diff --cached --check` passed with the isolated index.

| Candidate path | Git blob |
| --- | --- |
| `port/game/Combat.cpp` | `2e8a217a7773938ea36fffeffb05034f94437316` |
| `port/game/Combat.h` | `92aead2fd5c9c7d3b264a69b37a6bd18a1d9ee4e` |
| `port/game/LevelScene.h` | `34c90ced57c15b91ef86ea168ba2640bdfd471ef` |
| `port/game/PlayerMotion.cpp` | `8cea122bf0e2fbc01d1cfb31547ee3dd862a52bb` |
| `port/game/PlayerMotion.h` | `4d3cc7b0a646b4102c2f4aecd5ae12cd25f55689` |
| `port/linux/SDLRenderer.cpp` | `4e36e41febf12a1c6f4780ab6f470f8e70ac623a` |
| `port/linux/main.cpp` | `971aa7822dfe31355e1f31278f2f1c274c704172` |
| `port/render/Renderer.h` | `4bce78126a6f7838eaa0ffbaec9154755a37f9d2` |
| `port/tests/player_blood_blocked_rebound_cpu_test.cpp` | `c26ec1305f065c1737aa0c4623b4ff71f560b454` |
| `port/tests/player_blood_first_line_occlusion_cpu_test.cpp` | `aa60a59a9d464c3101377bff7261c40a60027244` |
| `port/tests/player_blood_impact_cpu_test.cpp` | `afb84be9b10f42a794cec705bbb1c18c5b1bda9a` |
| `port/tests/player_hit_blood_visual_test.cpp` | `32211ac6ebd58a220bd6fca8fca05e6ae718455f` |
| `port/tests/player_shot_launch_state_test.cpp` | `8519f294b56a3138b40b0c686e7fcb818c601612` |

`g++ -std=c++17 -fsyntax-only -I<snapshot>/port/compat` over snapshot `main.cpp`, `PlayerMotion.cpp`, `Combat.cpp`, and `LevelScene.cpp` **passed**. SDL source syntax was **not run**: host `pkg-config` did not provide both `sdl2` and `glew`. No GPU, Xorg, Docker, link, runtime, or full build was attempted. The prior Combat handoff reports all five focused fixtures passed against the live dirty worktree; this snapshot check does not re-claim those as staged-snapshot fixture results.

## Ownership and source disposition

All five owned dependency files were already dirty and source-backed by `RIGHT_STICK_MAIN_WIRING_IMPLEMENTATION_2026-10-22.md`, `RIGHT_STICK_IMPACT_RECOVERY_CPU_2026-10-22.md`, `STRAFE_BODY_CPU_IMPLEMENTATION_2026-10-22.md`, and `RIGHT_STICK_CAMERA_OFFSET_IMPLEMENTATION_GATE_2026-10-22.md`; no new implementation was required. The camera recovery duration is still the documented animation-finish approximation. The Combat/shot/blood source contract and focused fixture results are in `COMBAT_SHOT_BLOOD_INTEGRATED_2026-10-22.md`. A coordinating owner must explicitly include or separately land the SDL override, review the full mixed `main.cpp` diff and Combat ownership, rerun verification against the final staged tree, then decide the commit boundary. Shared dirty edits, checkpoints, and status files were preserved.
