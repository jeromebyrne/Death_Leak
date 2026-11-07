# Controller-strafe body CPU implementation (2026-10-22)

## Implemented

- `Renderer` exposes a non-pure raw `ControllerAxes()` DTO; SDL reads attached gamepad LX/RX/RY, while the existing strict ±8000 movement `KeyDown` gate is unchanged.
- `main.cpp` snapshots axes after event polling, stores last nonzero input movement direction separately from combat facing/velocity, detects the fresh airborne-to-grounded edge, and passes a newly cleared visual DTO only on unscripted active-play updates. Combat still receives ordinary facing; right-stick Y and fire do not change aim/projectiles.
- `LevelScene` loads only the authored body `StrafeForward`/`StrafeBack` XML sequences in order, using its existing texture cache and metadata. Grounded side-clear motion above the strict `abs(vx)>3` boundary selects forward/back from the DTO after melee, roll, water, dash, air and wall-slide priorities. Body and transient arm use aim-facing flip; aim-line offset/Y flip retain combat facing. Running-only spray remains Running-only.
- Each strafe sequence has a mutable XML-initialized 30 FPS rate. Animation advances at most once using the prior rate and keeps excess elapsed time, then stores the dry non-sprint Player `2.65` rate. Only a name transition restarts frame/clock; existing Running rate is untouched.

## Host-only CPU verification

Compiled the focused fixture to `/tmp/deathleak_strafe_body_cpu_test` with C++17, `-UNDEBUG`, `LevelScene.cpp`, and TinyXML sources; **PASS**. The fixture checks strict raw-axis edges and per-call reset, both aim/travel signs, RY-activated RX=0/sign cases, roll/melee/landing suppression, all 20 XML frames in both directions, first-entry 30 FPS vs stored 47.7 FPS, retained clock excess and one-step maximum, transition/re-entry, grounded side collision and higher-priority body selections, stopped-motion exit, no strafe material spray, and aim-line facing independence.

Compiled/reran the existing `player_arm_parity_test`, `aim_line_parity_test`, and `footstep_material_spray_test` as standalone host executables under `/tmp`; **all PASS**. No shared build directory, Docker, GPU, Xorg, capture, commit, or push was used. `git diff --check` passed for the owned source files and fixture.

## Exact unverified gates and bounded limits

- No SDL controller hot-plug, modal/inactive runtime, physical `KeyDown` ±8000 boundary, or full game build/run was exercised. Source review confirms the existing SDL ±8000 predicate is unchanged. The renderer's merged keyboard/controller `KeyDown` seam cannot distinguish simultaneous keyboard motion from controller motion; source-only review does not establish that mixed-input priority.
- The just-fell gate is a one-tick airborne-to-grounded approximation because `PlayerMotion` lacks the original short/large impact recovery states. No sticky recovery or crouch priority parity is claimed. Dry non-sprint rate only: sprint, wet-last-frame max-only clamp, slow-motion multiplier, and `StrafeBack` per-frame skeleton pose are deferred until their states/render path exist.
- The focused fixture is intentionally standalone because `port/tests/integration.sh` is outside this lane's write ownership. The shared build/GPU lane must still run a normal build, full player-visual regression, integration, runtime controller entry/exit/disconnect, and any matched Windows visual comparison. Pixel parity additionally requires matched input, animation phase, XY/velocity, camera, fire/arm state, and simulation time.
