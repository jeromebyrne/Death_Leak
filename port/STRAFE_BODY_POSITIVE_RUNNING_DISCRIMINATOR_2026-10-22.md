# Strafe body positive Running spray discriminator — 2026-10-22

## Result

**PASS, standalone CPU-only** with assertions enabled. Added a positive control to `port/tests/strafe_body_cpu_test.cpp`; no runtime, integration, Makefile, GPU/Xorg, Docker, or shared build-output change. This fixture remains **unregistered**.

## Discriminator

A fresh `forest_exploration_1.xml` scene uses solid-line segment 0 (`grassground`) and loads its authored `Game/Media/rock_particle.png`. After `SetPlayerFootstepContact(0, playerX, playerY)`, a grounded non-strafe Running update at `vx=600` and `dt=.001` produces exactly one footstep spray, with the grass particle texture and three particles. Fifty grounded StrafeBack updates at `vx=-600`, `dt=.05` produce no further spray in the same scene. The existing separate strafe-only zero-spray check also passes.

This is a differential test of Running dispatch versus StrafeBack suppression under a valid contact/material, not a visual/GPU test. It does not verify real controller input, SDL, or all material variants.

## Source basis

- `Game/XmlFiles/levels/forest_exploration_1.xml` first `SolidLineStrip` (ID 3) has `material=grassground`; `Game/XmlFiles/materials.xml` maps `grassground`'s first particle texture to `Media\\rock_particle.png`.
- `Game/XmlFiles/animation/player_anim_default.xml` marks Running frame 000 `sfx_type="footstep"`.
- `port/game/LevelScene.cpp`: `EmitRunningFootstep` requires a marked frame, valid ground segment, non-water material and loaded texture; it creates three particles. `UpdatePlayerVisual` calls it only when target is `Running` and frame differs from `lastRunningFrame_` while grounded. A fresh scene starts `lastRunningFrame_` at invalid and Running enters frame 0, making the positive event deterministic.

## Verification

From repository root, compiled the fixture and current port sources into an isolated `/tmp/strafe-running-cpu-XXXXXX` directory:

```sh
verify_dir=$(mktemp -d /tmp/strafe-running-cpu-XXXXXX)
g++ -O2 -g -std=c++17 -Wall -Wextra -Wpedantic -UNDEBUG -Iport/compat \
  port/tests/strafe_body_cpu_test.cpp port/game/LevelScene.cpp port/game/PlayerMotion.cpp \
  Game/tinyxml/tinyxml.cpp Game/tinyxml/tinyxmlerror.cpp \
  Game/tinyxml/tinyxmlparser.cpp Game/tinyxml/tinystr.cpp \
  -o "$verify_dir/strafe_body_cpu_test"
env -u DEATHLEAK_RENDER_HIDE_XML_IDS "$verify_dir/strafe_body_cpu_test"
```

Exit **0**; stdout: `PASS: strafe raw-input gate, body selection/flip, clock, priorities, spray, aim-line`. Compiler warnings were limited to four existing TinyXML diagnostics (one `-Wparentheses`, three `-Wimplicit-fallthrough`); none from the fixture or port source. No commit/push performed. Handoff: owner can review the two scoped files, then separately decide on integration gate registration.
