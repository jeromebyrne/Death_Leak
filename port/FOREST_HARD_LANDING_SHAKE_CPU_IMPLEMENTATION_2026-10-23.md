# Forest hard-landing camera shake — CPU implementation (2026-10-23)

## Scope and source contract

`Game/src/GameObjects/Character.cpp:124-135,198-212,337-378` starts `Camera2D::DoMediumShake()` for player downward-dash solid contact, a non-water ordinary fall of at least 700 units, and a buffered roll on such a fall. `Game/src/Core/Camera2D.cpp:120-166,282-308` specifies 15 world units over 0.30 seconds, no retrigger while active, per-axis random signs on followed axes, and a view matrix built from the **pre-jitter** stored camera position. `Game/src/program.cpp:158-175` advances total time *after* game update. The forest-1 camera follows X but not Y (`forest_exploration_1.xml:33-46`).

## Exact software files

- `port/game/PlayerMotion.h/.cpp`: one-shot `HardLandingEvent` for downward dash, >=700 ordinary drop, or successful buffered roll on that drop. Reset and consume semantics prevent stale/repeated events. Existing soft/hard recovery and material spray remain intact; an already-active roll does not independently trigger shake.
- `port/game/CameraShake.h` (new): isolated state machine for 15/0.30 medium shake. Injectable signs expose deterministic CPU testing. Update returns both effective view and post-jitter stored coordinates; retriggers are ignored while active; zero jitter at expiry, on unfollowed axes, or before trigger. `WillJitter` prevents consuming random signs on inactive/expired updates.
- `port/linux/main.cpp`: starts shake after contact/buffered-roll processing; follows/clamps before the event as before; samples the camera at the pre-increment game-update clock; feeds the returned **view** coordinates to scene/painter drawing while retaining post-jitter stored XY for the next follow update. Forest Y is unjittered. HUD drawing remains screen-space and unchanged. Camera state resets on level/restart transitions. No renderer, HUD, XML, asset, status, or integration-test file was edited.
- `port/tests/camera_shake_cpu_test.cpp` and `port/tests/hard_landing_event_cpu_test.cpp` (new): deterministic camera trace and source-threshold/contact fixtures.

## CPU trace and verification

The helper trace logs event/frame, base XY, active clock remaining, stored XY, effective view XY, and fixed HUD XY. Control at frame 0 remains `(100,200)`. Trigger frame 60 at t=1.0 sees view `(100,200)` and stored `(115,200)`; after a modeled follow step, frame 61 at t=1.1 sees view `(101.5,200)` and stored `(91.5,200)` with opposite sign; expiry frame 78 at t=1.3 has zero new jitter. Assertions cover no pre-trigger/expiry injection, no active retrigger, <=15-unit per-update displacement, no forest Y jitter, and view/stored distinction. The fixed HUD coordinates model the unchanged screen-space draw boundary; no rendered HUD pixels were tested.

`hard_landing_event_cpu_test.cpp` loads actual forest-1 ground with a null renderer and asserts authored-entry suppression, 699.875-unit no event, 700-unit one-shot ordinary event, buffered-roll event, and short-drop downward-dash event. Random signs are injected for tests; no claim of bit-identical Windows RNG or matched source runtime trace.

Commands: `g++ -std=c++17 -O0 -Wall -Wextra -pedantic port/tests/camera_shake_cpu_test.cpp -o /tmp/death-leak-camera-shake-cpu && /tmp/death-leak-camera-shake-cpu`; `g++ -O2 -std=c++17 -UNDEBUG -Iport/compat port/tests/hard_landing_event_cpu_test.cpp port/game/LevelScene.cpp port/game/PlayerMotion.cpp Game/tinyxml/tinyxml.cpp Game/tinyxml/tinyxmlerror.cpp Game/tinyxml/tinyxmlparser.cpp Game/tinyxml/tinystr.cpp -o /tmp/death-leak-hard-landing-event-cpu && /tmp/death-leak-hard-landing-event-cpu`. Both PASS. The prior `right_stick_camera_cpu_test.cpp` regression also PASS with the modified `PlayerMotion.cpp`. Scoped `git diff --check` PASS. Docker `deathleak-linux-build:bookworm` read-only source-mount `make -e -C port TARGET=/out/deathleak-linux -j2` PASS, with only existing TinyXML warnings; isolated output `/tmp/death-leak-shake-build/deathleak-linux`. This is a current working-tree compile including unrelated dirty renderer content, not a clean-HEAD/GPU acceptance build.

Software commit `05cc746` (`Port hard-landing camera shake with temporal CPU tests`) contains exactly the six source/test paths above and was pushed to `origin/feature/linux-renderer-foundation`. This sole report remains local/untracked, separate from the software commit.

## Ownership and next GPU gate

Before editing, `git status --short` showed `port/linux/main.cpp` and `PlayerMotion` clean. HUD transition receipt `HUD_TRANSITION_CURRENT_TREE_GPU_INTEGRATION_2026-10-23.md` explicitly releases HUD software/build/GPU ownership after commit `9ba65c3`; gameplay-film implementation is committed as `c0c87ff` and `GAMEPLAY_HUD_FILM_GPU_REVIEW_2026-10-23.md` releases its physical GPU lane. The current main diff was therefore a clean scoped addition, not an overwrite of dirty HUD work. Renderer, integration script, coordinator files, and many reports remain unrelated/dirty and were preserved.

Next gate: acquire an exclusive physical GPU lane and a qualifying forest-1 state/route (unlocked dash into known solid or measured >=700 fall), capture lossless frame/event/view/stored-position trace against an unshaken control, then obtain an original Windows trace/image with matched state/time/modifier. Only then assess visible foreground motion and pixel parity. No visual acceptance is claimed.
