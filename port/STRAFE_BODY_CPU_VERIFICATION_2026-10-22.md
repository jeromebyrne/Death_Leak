# Strafe body standalone CPU verification — 2026-10-22

## Result

**PASS** for the current focused fixture, with assertions enabled. Compiled and executed from the repository root against current `LevelScene.cpp`, `PlayerMotion.cpp`, TinyXML, and authored `Game` assets. The executable was isolated at `/tmp/strafe-body-cpu-Pu9yXc/strafe_body_cpu_test`; no `port/build`, shared Makefile, Docker, GPU/display, or ffmpeg path was used. Exit status: **0**.

Observed stdout:

```text
PASS: strafe raw-input gate, body selection/flip, clock, priorities, spray, aim-line
```

The compiler emitted only pre-existing-style TinyXML warnings from `tinyxmlparser.cpp`: one `-Wparentheses` at line 358 and three `-Wimplicit-fallthrough` diagnostics at lines 114, 118, and 122. No strafe fixture or port source warning appeared. These warnings did not prevent compilation.

## Reproduction

From `/srv/openclaw/workspace/death_leak_analysis/source`:

```sh
verify_dir=$(mktemp -d /tmp/strafe-body-cpu-XXXXXX)
g++ -O2 -g -std=c++17 -Wall -Wextra -Wpedantic -UNDEBUG -Iport/compat \
  port/tests/strafe_body_cpu_test.cpp port/game/LevelScene.cpp port/game/PlayerMotion.cpp \
  Game/tinyxml/tinyxml.cpp Game/tinyxml/tinyxmlerror.cpp \
  Game/tinyxml/tinyxmlparser.cpp Game/tinyxml/tinystr.cpp \
  -o "$verify_dir/strafe_body_cpu_test"
env -u DEATHLEAK_RENDER_HIDE_XML_IDS "$verify_dir/strafe_body_cpu_test"
```

`-UNDEBUG` keeps the fixture's `assert` checks live. Unsetting the inherited hide-ID diagnostic prevents unrelated render filtering; the fixture itself sets its only-ID filter.

## Evidence and scope

- Read `port/STRAFE_BODY_FIXTURE_FINAL_STATIC_QA_2026-10-22.md`, the repaired fixture, and current `LevelScene` strafe resolver, sequence selection/clock, spray dispatch, and draw-facing code. The focused source/test signatures match and the standalone link/run passed.
- The fixture observes strict raw-stick eligibility and no-sticky gating; authored StrafeBack/StrafeForward frame selection, flips and all 20 frames in both directions; stored frame-rate timing at high and low speed; higher-priority state fallbacks; absence of Running-only footstep spray while strafing; and unchanged aim-line destination/flip under strafe. These are fixture claims demonstrated by the passing assertions, not a full integration or Windows visual-parity result.
- The fixture is still absent from `port/Makefile` and `port/tests/integration.sh`; it is a **standalone CPU pass**, not a registered gate. It does not exercise real SDL input/disconnect, positive Running spray, wet/sprint/slow-motion variants, impact recovery, or GPU/pixel output.

## Scoped handoff

Recommend the owner review the current uncommitted strafe implementation and fixture, then stage **only intended strafe hunks/files** (including `port/tests/strafe_body_cpu_test.cpp` and this report) for a scoped commit and push. `LevelScene.cpp` and `.h` are modified and the fixture is untracked; the working tree also contains many unrelated modified/untracked files, including `port/tests/integration.sh`. Do not use `git add -A`, and do not present this standalone pass as a registered integration gate. Gate registration can follow separately if desired, ideally with a positive Running spray discriminator. This verification did not commit, push, or edit source/tests/shared checkpoint.
