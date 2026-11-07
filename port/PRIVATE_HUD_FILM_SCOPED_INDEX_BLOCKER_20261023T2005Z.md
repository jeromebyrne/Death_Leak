# Private HUD-film scoped-index integration gate — 2026-10-23 UTC

## Verdict

**Do not commit or push `port/linux/main.cpp` alone yet.** The file's three film-selector hunks remain internally coherent and the focused selector fixture passes, but a frozen clean-index-equivalent build of `HEAD` plus **only** this file fails on a pre-existing, unrelated `CameraShake` API dependency. No tracked source, index, commit, push, shared report, or GPU state was changed by this gate. The current live `port/game/Hud.cpp` became dirty during this task, so all build inputs were frozen before compiling.

## Exact scoped evidence

- `HEAD=515fe23`; working `main.cpp` SHA-256 `fe0e9009f2e83df9517514802a21274ca29a268c045d6ffb10a213501b63bf30`; diff is still 30 additions/13 deletions in three film-only hunks. `git diff --check -- port/linux/main.cpp` passed; index was empty before/after. The helper and distinct ordinary/final film state were reviewed against `Game/src/UI/UIAnimatedSprite.cpp:61-77` and the two XML HUD definitions. The ordinary/final selectors use independent retained state, one-step strict 11-FPS integral-ms thresholds and the authored four assets. No matched original-Windows frame/phase was available.
- `python3 port/tests/gameplay_film_first_active_frame_private_cpu_test.py` passed, extracting/compiling the production helper and checking first 90/91-ms boundary, one-step late updates, wrap, assets and ordinary draw order. The previous controlled-index physical NVE7 A/B in `PRIVATE_SEA2_FINAL_FILM_CONTROLLED_INDEX_GPU_GATE_20261023.md` remains valid for its prior dirty-tree binary only, not for this clean scoped snapshot.
- Frozen build input at ignored `port/build/private-hud-film-scoped-20261023/snapshot/`: `git archive HEAD` of the Makefile/source dependency directories, then exactly this `main.cpp` copied (same SHA-256). Docker image `deathleak-linux-build:bookworm` built to unique ignored `.../out/` with log `.../build.log`. First attempt had only a read-only mount setup failure (missing precreated `/src/port/build`); that directory was created in the snapshot and the actual compiler run proceeded.
- Scoped compiler fails at `linux/main.cpp:713`: `CameraShake` has no `Start` member. The call is **already in HEAD** at `port/linux/main.cpp:697`, while HEAD `port/game/CameraShake.h` exposes only `StartMedium`. A separate unstaged dirty `CameraShake.h` change adds `Start(float,float,float)`; incorporating it would exceed this task's allowed single-file commit. This is independent of the HUD-film diff. See `build.log` for exact compiler diagnostic. No scoped binary exists, so no valid new scoped active-gameplay GPU capture was attempted.

## Release / next owner action

The main-only commit cannot be represented as a verified buildable change against current HEAD. Once the CameraShake API mismatch is committed/resolved by its owner (or a coherent authorized multi-file integration plan is chosen), re-freeze HEAD plus the identical `main.cpp` diff, rerun the clean scoped build and selector fixture, then recheck the live diff/index before any exact-file stage/commit/push. Do not infer original-Windows/Jerome acceptance from the prior Linux index A/B.

**Exclusive Docker build, physical Xorg `:0`/NVE7 GPU, capture, and `port/linux/main.cpp` ownership are released.** No Death Leak build/game/GDB process from this task remains. The other worker's `port/game/Hud.cpp` is untouched.
