# Right-stick live-loop wiring — 2026-10-22

Scoped CPU/source handoff; no runtime, GPU, display, or pixel-parity acceptance claimed.

- `port/linux/main.cpp` now owns a `RightStickCameraTarget` initialized from the loaded scene's authored offsets. Successful menu New Game, death restart, door, boat, and trigger replacements reset it immediately from the new scene. Story-completion scene clear also resets it; failed loads do not.
- The previous manual follow/clamp block now calls `FollowCameraWithTarget` with the same zoom, axis flags, bounds, player position, enemy-dependent X lag, and prior camera state. Scripted-camera override still follows the helper. The helper consumes the **previous** input tick's target before motion advances.
- In living gameplay only, after motion, any buffered roll, combat, and contact damage, the sampled controller axes update the **next** target. The falling/X-hold argument is `motion.ImpactRecovering()`; rolling and melee are passed independently. Menu, modal, dead/restart, final scene, scripted, and simulation-only paths never write the target. The helper itself handles disconnected and neutral sticks, SDL right-Y sign, and Y-before-X updates. No dependence on strafe-visual activation was added.

Verification: `g++ -std=c++17 -fsyntax-only -Iport/compat port/linux/main.cpp` and `git diff --check -- port/linux/main.cpp` passed. Source review found six reset points (five successful scene replacements plus story clear) and one gameplay input-update point after follow. The pre-existing, uncommitted strafe-body edits in `main.cpp` were preserved; this shared worktree was not committed or pushed. The right-stick helper, `PlayerMotion`, fixture, renderer, and shared checkpoint/status were not edited by this slice.

Remaining handoff: build/run and controller/display acceptance belong to the designated integration/GPU owner. The `PlayerMotion` impact-recovery expiry remains an approximate CPU timing contract, not source-frame-exact Windows animation completion. Scripted/simulation camera behavior stays at authored offsets.
