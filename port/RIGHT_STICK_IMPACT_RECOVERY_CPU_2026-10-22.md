# Right-stick impact recovery CPU slice — 2026-10-22

Scope: `PlayerMotion` recovery state and motion-driven right-stick CPU assertions only. No live-loop wiring, GPU/display capture, integration script, renderer, or checkpoint changes.

## Implemented

- `PlayerMotion::Recovery()` exposes explicit `None` / `Soft` / `Hard`; `ImpactRecovering()` is the Boolean camera-gate input. Reset clears state. Ordinary in-flight movement and initial authored placement do not set it.
- On a genuine new solid landing, the retained airborne apex classifies `(300, 700)` units as Soft and `>=700` as Hard, excluding water and an existing roll. Downward-dash contact promotes to Hard independent of drop distance; the pre-contact dash value is retained before the existing ground-clear. A successful roll, including the currently post-update buffered-roll call, clears either recovery state. Hard recovery also clears on side collision.
- Recovery persists across grounded updates. Without a motion-layer animation-finished callback, expiry approximates the authored impact sequences: six frames at 30 fps for Soft and eleven frames at 15 fps for Hard (player animation XML). This is an explicit timing approximation, not frame-exact Windows animation parity. The landing update starts a full recovery interval; subsequent updates advance it.
- The existing right-stick fixture now drives its `falling` parameter from `PlayerMotion::ImpactRecovering()` for Soft and Hard contacts, asserting X hold/Y update, neutral reset, ordinary post-expiry X update, threshold boundaries, reset, buffered-roll cancellation, and pre-contact versus post-contact dash behavior.

## Verification

Focused host-only command (no game executable, GPU, display, or shared build output):

```sh
g++ -O2 -std=c++17 -UNDEBUG -Iport/compat port/tests/right_stick_camera_cpu_test.cpp port/game/LevelScene.cpp port/game/PlayerMotion.cpp Game/tinyxml/tinyxml.cpp Game/tinyxml/tinyxmlerror.cpp Game/tinyxml/tinyxmlparser.cpp Game/tinyxml/tinystr.cpp -o /tmp/right-stick-impact-cpu/right_stick_camera_cpu_test
env -u DEATHLEAK_RENDER_HIDE_XML_IDS /tmp/right-stick-impact-cpu/right_stick_camera_cpu_test
```

Passed after the final source cleanup. `git diff --check` passed for scoped tracked sources. No commit made.

## Exact files touched

- `port/game/PlayerMotion.h`
- `port/game/PlayerMotion.cpp`
- `port/tests/right_stick_camera_cpu_test.cpp` (already untracked in the shared worktree before this slice; preserve the existing helper/visual tests)
- `port/RIGHT_STICK_IMPACT_RECOVERY_CPU_2026-10-22.md` (this report)

## Next wiring step / remaining limits

The `main.cpp` owner should pass `motion.ImpactRecovering()` as the fourth/falling argument to `RightStickCameraTarget::Update`, after `motion.Update` and any same-frame `TryBufferedRoll`, retaining follow-before-input and all scene-reset points in `RIGHT_STICK_MAIN_WIRING_MAP_2026-10-22.md`. This slice does not wire that file, so it does not establish live camera parity. Source-exact recovery expiry still requires an animation-finished signal in motion/visual integration; the approximation here is intentionally bounded and documented. Water and already-rolling classification are implemented but not exercised by this focused fixture's selected dry-floor landing route.
