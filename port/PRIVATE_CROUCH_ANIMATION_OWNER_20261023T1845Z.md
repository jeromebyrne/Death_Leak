# Private grounded crouch animation owner — 2026-10-23 18:45Z

## Result

Implemented Forest-1 Player ID2's authored grounded `Crouch` and `CrouchUp` body sequences in the Linux port. The XML loader now loads their individual `duck_default_000`–`005` textures. Grounded crouch selects the six-frame nonlooping 40-fps sequence over running/idle and holds frame 005. Release marks the `CrouchUp` edge, plays the six reverse-authored textures at 40 fps, then returns to Still; running/air/roll/combat interrupt stand-up as in source branch priority. The body clock still consumes the preceding rendered modified delta and advances at most one frame per update.

`--script C:n` requests crouch in deterministic input; ordinary Down/S (including mapped down pad) requests grounded crouch when not melee/rolling/wall-side. That request stops horizontal acceleration through the existing motion input. A grounded crouch request suppresses the port's overlapping Down-to-fire mapping; right-stick fire remains available. Airborne Down fire remains unchanged. This resolves the keyboard S pose gap without claiming full controller down-stick parity (renderer input abstraction is outside this owner's files).

## CPU verification

- `port/tests/crouch_animation_cpu_test.cpp`: PASS. Forest-1 ID2 render-command texture paths verify 000→005 hold, 005→000 stand-up, 40-fps cadence using prior delta, crouch priority over residual run speed, and running/roll interruption.
- `fresh_save_intro_animation_test.cpp`: PASS (existing intro body clock).
- `player_arm_parity_test.cpp`: PASS (existing Still/Running arm composition).
- `g++ -std=c++17 -fsyntax-only -Iport/compat port/linux/main.cpp`: PASS.
- `git diff --check` on owned files: PASS.

No GPU/Xorg/Wine/full build or Windows matched-frame run was performed. Pixel-level Windows/Linux acceptance remains with the separate matched-frame owner. Shared status files and unrelated dirty files were untouched.

## Release

Scoped software and this report only; no ownership of Windows capture or display lane. Commit/push disposition recorded in completion handoff.
