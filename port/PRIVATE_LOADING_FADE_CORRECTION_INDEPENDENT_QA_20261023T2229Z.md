# Forest→Waterfall loading-fade correction — independent source-contract QA

Snapshot: dirty `port/linux/main.cpp` as read on 2026-10-23; exclusive implementation worker may change it after this review. This is source-only QA; no build, game run, GPU work, or shared status/index changes.

## Finding

No actionable defect in the newly changed FadeOut gameplay-update or opaque-hold exit boundary in this snapshot. Lane released to exclusive implementation worker for its own verification and integration; this note is not a runtime/GPU sign-off.

## Exact contract evidence

- Original `Game/src/Game.cpp:176-179,204-215,253-255` updates game objects and other gameplay before `UIManager::Update`; a pause-state change by the loading UI therefore takes effect on gameplay the *following* frame. Original `Game/src/UI/UIGameLoading.cpp:76-92,96-103` calls `FinishedLoading()` from the opaque-hold branch, unpausing there, and only reaches the `mHasStartedFadeOut` branch on a later UI update. Thus the hold-exit frame remains gameplay-paused and fully opaque; the next frame both updates gameplay and begins visual reveal.
- Linux snapshots pause on frame entry at `port/linux/main.cpp:568-576`, executes the hold branch at `:686-718`, and switches to FadeOut only after the hold's `> 0.05f` condition at `:712-718`. The hold-exit frame stays in the paused branch and forces full alpha at `:1535-1545`. The next frame enters gameplay at `:719-730`; weather and scene/player visual updates use that frame-entry pause snapshot at `:1332-1368`. This matches the source boundary.
- Original `Game/src/UI/UIGameLoading.cpp:105-129` starts the fade-out clock during `Load()`, before the minimum hold ends; `:84-92` uses that original clock for the first reveal alpha. Linux resets its clock at scene installation at `port/linux/main.cpp:690-711`, accumulates it through the hold at `:686-689`, preserves it across hold exit at `:712-718`, and advances it for active FadeOut at `:723-730`. This retains the elapsed hold time instead of restarting fade-out at hold exit.
- The focused regression `port/tests/private_loading_fade_cpu_regression_20261023.py:73-100` checks the connected Forest trigger, opaque paused hold-exit frame, gameplay/weather/scene/player updates under the first revealing FadeOut frame, and gameplay continuity through splash retirement. Its observer taps are in `port/linux/main.cpp:573-577,720-721,1335-1337,1353-1365,1542-1544,1553-1565`. Regression was inspected only, not executed in this lane.
