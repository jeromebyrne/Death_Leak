# Right-stick aim/fire CPU implementation — 2026-10-22

Scope: live Linux input/combat wiring only. This slice did not alter impact-recovery control lockout, renderer, camera helper, shared status, builds, or GPU/display state.

## Changes

- `port/linux/main.cpp`: in living gameplay, the already-sampled connected controller's right X or world-up-converted right Y activates fire when either axis is strictly outside the Windows 8689 deadzone. Both raw components are passed to combat for normalization. Keyboard/controller Down and scripted fire remain available; scripted/simulation-only runs ignore stick input. Fire eligibility bookkeeping now sees stick attempts. The pre-existing uncommitted camera and strafe wiring remains intact.
- `port/game/Combat.{h,cpp}`: `Update` accepts optional aim components, normalizes them for player-knife velocity, and falls back to horizontal facing when absent. The existing 1200-unit speed, damage, cooldown, burst, water/roll/melee/health gates, and player-shot collection remain. A player knife records its launch direction so its contact spray reverses the original direction rather than its gravity-altered current velocity. The optional arguments preserve existing CPU-test and other call sites.

## Source contract and verification

Original `Game/src/InputManager.cpp` `ProcessAimDirection_gamepad` and `ProcessPrimaryWeapon_gamepad` use the same strict right-axis deadzone, normalize the full X/Y pair, and continuously request `Player::FireWeapon` while active. Original `Game/src/GameObjects/Player.cpp` uses that vector for the projectile's direction and picks a left launch offset for zero X. The Linux gameplay tick computes aim/fire before `Combat::Update`, then updates the next camera target after combat; the camera still follows the prior target before movement.

Passed: `g++ -std=c++17 -fsyntax-only -Iport/compat port/game/Combat.cpp port/linux/main.cpp`; `git diff --check -- port/game/Combat.h port/game/Combat.cpp port/linux/main.cpp`. No new test file was necessary for this bounded wiring; no build, controller runtime, GPU, or pixel-parity run was performed.

## Remaining gaps / handoff

Live controller verification and integrated build belong to the build/GPU owner. The separately identified impact-recovery movement/jump lockout remains untouched. Existing port divergences in projectile origin (including crouch/airborne offsets), wall-side fire gating, and trajectory/visual details are outside this slice; this change implements right-stick activation and aim vector through the existing Linux projectile path, not full Windows projectile parity. No commit or push was made.
