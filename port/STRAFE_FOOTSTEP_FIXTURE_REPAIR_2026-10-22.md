# Strafe-footstep broad fixture repair — 2026-10-22

## Scope

Edited only `port/tests/strafe_body_cpu_test.cpp` and this report. Left the concurrent `LevelScene` implementation, focused fixture, build/GPU/Docker resources, coordinator files, and unrelated dirty state untouched. No commit or push.

## Source-backed expectation

`Game/XmlFiles/animation/player_anim_default.xml` marks StrafeBack sequence indices 9 and 19 as `sfx_type="footstep"` (displaying reverse-order textures 010 and 000). `port/game/LevelScene.cpp:1124-1145` advances at most one frame per update and dispatches the shared horizontal footstep guard for Running and both strafe sequences; `:1023-1055` emits three rock particles for the forest grass contact, offset by movement direction. Accordingly, both old assertions that 50 grounded StrafeBack ticks add zero sprays were obsolete.

The broad fixture now checks the count on every tick: the first 0.05-second tick enters index 0 and advances to 1, so marked indices recur on ticks 9, 19, 29, 39, and 49, totaling five sprays. At each marked tick it verifies rock texture, three-particle payload, leftward-movement origin (`x - 12`), and positive particle X direction despite opposite aim in the first scene. The fresh-scene Running frame-0 positive control remains, followed by the same exact StrafeBack count/payload checks. All unrelated body, timing, priority, aim-line, and low-speed assertions remain unchanged.

## Host-only verification

From repository root, `g++` was present. Compiled each of `strafe_body_cpu_test` and `strafe_footstep_cpu_test` into a private `/tmp/deathleak-strafe-fixture-*` directory with:

```sh
g++ -O2 -g -std=c++17 -Wall -Wextra -Wpedantic -UNDEBUG -Iport/compat \
  port/tests/$fixture.cpp port/game/LevelScene.cpp port/game/PlayerMotion.cpp \
  Game/tinyxml/tinyxml.cpp Game/tinyxml/tinyxmlerror.cpp \
  Game/tinyxml/tinyxmlparser.cpp Game/tinyxml/tinystr.cpp -o "$fixture_dir/$fixture"
env -u DEATHLEAK_RENDER_HIDE_XML_IDS "$fixture_dir/$fixture"
```

- Broad fixture: **PASS** — `PASS: strafe raw-input gate, body selection/flip, clock, priorities, spray, aim-line`.
- Focused fixture: **PASS** — `PASS: authored strafe footstep frames, shared guard, movement direction, contact/water, Running`.
- Both builds exited successfully. Compiler warnings were confined to pre-existing `Game/tinyxml/tinyxmlparser.cpp` parentheses/fallthrough diagnostics.
- `git diff --check -- port/tests/strafe_body_cpu_test.cpp`: **PASS**.

No full integration suite was run: it invokes the shared game binary/rendering path and another worker owns physical GPU/Docker/build activity. These results establish the scoped CPU fixture gate only, not renderer or full integration acceptance.
