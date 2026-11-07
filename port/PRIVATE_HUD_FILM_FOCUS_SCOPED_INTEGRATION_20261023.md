# Private HUD-film / focus scoped integration handoff — 2026-10-23

## Disposition

PASS for a clean, scoped Linux software build and focused CPU verification. Commit `e10084a6b13019c57ca1d96d886fd56fe624f644` (`Align HUD film timing and focus meter UV with original UI`) contains **only** `port/linux/main.cpp` and `port/game/Hud.cpp`; pushed to `origin/feature/linux-renderer-foundation`, verified by `git ls-remote`. The prior CameraShake API blocker is resolved in parent HEAD `92562c4`. All other dirty tracked and untracked files, including shared `PORT_WORK_STATUS.md`, were preserved and unstaged.

## Source and frozen verification

At HEAD `92562c4`, live diffs were exactly three coupled film-selector hunks in main.cpp (30 additions / 13 deletions) and the one-field focus-fill `flipY=true` hunk in Hud.cpp (1/1). SHA-256 before snapshot and immediately before staging: main `fe0e9009f2e83df9517514802a21274ca29a268c045d6ffb10a213501b63bf30`; Hud `81df0b65c7d69741c98b0816196e5bad6e749048e02ba5bf708fedbf4ae9757c`. `git diff --check` and staged `git diff --cached --check` passed; index was empty before staging and contained exactly those two paths after staging.

Frozen `/tmp/deathleak-hud-scope-20261023` was created from `git archive HEAD Game/tinyxml port`, then overlaid with exactly these two files (matching hashes). `docker run --rm -v /tmp/deathleak-hud-scope-20261023:/src -w /src/port deathleak-linux-build:bookworm make -j2` exited 0, producing an ELF `port/build/deathleak-linux`; log is `/tmp/deathleak-hud-scope-20261023/build.log`. Thus no unrelated dirty renderer, particle, integration-test or status file entered this build.

Focused tests passed: `python3 port/tests/gameplay_film_first_active_frame_private_cpu_test.py` (selector boundary, assets, draw order); `g++ -std=c++17 -O0 -g -UNDEBUG port/tests/focus_bar_vertical_uv_private_cpu_spy_20261023.cpp port/game/Hud.cpp -o /tmp/deathleak-focus-uv-scope-test` followed by that binary (positive focus fill and non-focus flip checks). These CPU test sources were not staged. Existing prior private reports document physical NVE7 controlled sea_2 film-index causality and focus A/B/A (762 changed pixels confined to fill ROI), but this integration performed **no new GPU run** and does not upgrade them to matched-Windows parity.

## Ownership and remaining gate

No Death Leak build/game/GPU process from this lane remains. Scoped source/index/Docker build ownership is released; Xorg/GPU was not touched. Matched original-Windows HUD frame/phase and Jerome's visual acceptance of active-gameplay footage remain open. Do not describe Linux build/CPU/previous NVE7 causality as cross-platform pixel parity.
