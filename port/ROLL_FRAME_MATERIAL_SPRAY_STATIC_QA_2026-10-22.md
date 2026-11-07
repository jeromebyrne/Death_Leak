# Roll-frame material spray — static QA (2026-10-22)

Scope: read-only comparison of current port implementation and CPU fixture with `Game/src/GameObjects/Character.cpp` and authored `player_anim_default.xml`. No build, integration, GPU run, or source edit.

## Finding — scripted leftward rolls spray in the wrong direction

**Concrete parity bug (scripted playback):** `port/linux/main.cpp:456` initializes `PlayerStrafeVisual` with its default `movementDirection = 1` (`port/game/LevelScene.h:77`). A scripted `L`/`N`/`Q`/`Z` updates the separate `movementDirection` and `facing` variables at `main.cpp:611-613`, but the DTO is only resolved/populated when `script.empty()` at `main.cpp:671-673`. The new Roll path calls `EmitRunningFootstep(sequence, strafe.movementDirection)` at `port/game/LevelScene.cpp:1146-1149`. Thus on an authored marked Roll frame in a scripted leftward roll, it emits at `x + 12` with a leftward `directionX` base instead of the source `m_direction.X = -1` origin `x - 12` and rightward base. Original source uses `m_direction.X` for both at `Game/src/GameObjects/Character.cpp:1149-1151`. This can contaminate scripted visual captures even though interactive input fills the DTO.

**Missing CPU edge:** `port/tests/roll_frame_material_spray_test.cpp:37-41` explicitly supplies `{false, 1, direction}` with `direction = -1`; this proves the emitter accepts a leftward DTO, not that the production scripted handoff supplies one. Add a scripted left-roll call-site/DTO assertion when repairing the handoff. Avoid changing Roll to `playerFacing_` globally: the source effect follows movement direction, and interactive strafe/aim facing can differ.

## Checked parity / limits

- XML Roll marks zero-based frames 10 and 16 (`Game/XmlFiles/animation/player_anim_default.xml:987,1017`); the implementation reads `sequence.footstepFrames[playerFrame_]`, and the fixture reaches both.
- Source invokes the effect after every Roll `Animate()` (`Character.cpp:875-900`) and compares to the last *running* frame (`Character.cpp:1119-1124`); it does not stamp that guard in the Roll branch. Current port's held-frame repeat and cross-sequence numeric guard follow that behavior. The fixture covers both.
- Material/solid-line filtering and exact source collision-boundary timing were not established by this static audit. No runtime or visual-acceptance claim.
