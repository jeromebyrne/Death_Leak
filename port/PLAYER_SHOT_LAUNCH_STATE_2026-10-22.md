# Player shot launch-state CPU correction — 2026-10-22

## Implemented

- Preserved the pre-existing uncommitted right-stick aim, camera, spray, and other shared edits in `Combat.cpp`, `Combat.h`, and `main.cpp`; changed only the player-shot launch-state path.
- `Combat::Update` now accepts an optional `PlayerShotLaunchState` and initializes player knives at `y + (crouching ? 0 : 25) + (ascending ? 65 : descending ? 40 : 0)`. This retains the original `Player::FireWeapon` crouch offset and ascending-before-descending branch order. Existing ±10 X, normalized aim, velocity, cadence, gates, texture, and contact path were not changed.
- The live main loop passes the state after `PlayerMotion::Update`: grounded Down input supplies crouch if not rolling/melee or wall-blocked; non-grounded/non-wall/non-water nonzero vertical velocity supplies ascending above −30 world units/s (the original −0.5 unit/tick threshold) or descending otherwise. No scripted-input command was reinterpreted as crouch. Airborne launch clears crouch as `Character::Update` does. Since the current renderer's `Down` key already means fire and exposes no separate left-stick Y, controller crouch via left-stick down is not independently wired in this slice.

## Evidence and verification

- Original: `Game/src/GameObjects/Player.cpp:424-444`; `Game/src/GameObjects/Character.cpp:290-328`; input gating at `Game/src/InputManager.cpp:146-159,649-662`.
- Focused `port/tests/player_shot_launch_state_test.cpp`: passed after compiling with host `g++`, `Combat.cpp`, `LevelScene.cpp`, and TinyXML into `/tmp`. Covers standing +25, crouch +0, ascending +90, descending +65, both crouch/air combinations, both flags' branch priority, and left/right horizontal and diagonal aim with unchanged ±10 X and 1200-speed velocity.
- `g++ -std=c++17 -fsyntax-only -Iport/compat port/linux/main.cpp`: passed. `git diff --check` on owned sources: passed.
- No full build, display/GPU execution, Windows same-state trace, commit, or push. This proves the bounded CPU launch-center contract, not rendered pixel parity.

## Handoff

Owned changes are limited to `port/game/Combat.cpp`, `port/game/Combat.h`, `port/linux/main.cpp`, `port/tests/player_shot_launch_state_test.cpp`, and this report. Existing shared modifications remain intact. Later GPU review should use a controlled airborne shot; full source input parity still needs an independent left-stick-Y crouch route outside this ownership boundary.
