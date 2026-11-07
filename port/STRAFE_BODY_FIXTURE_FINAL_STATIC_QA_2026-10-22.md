# Strafe body fixture — final static QA (2026-10-22)

Scope: read-only review of `STRAFE_BODY_CPU_FIXTURE_REPAIR_2026-10-22.md`, `tests/strafe_body_cpu_test.cpp`, relevant `LevelScene` code, authored XML/assets, and Makefile. This report is the only file written. No compile, test run, Docker, or GPU work.

## Verdict

**Ready for a deferred standalone CPU compile/run, but not yet a registered or executed gate.** No evident C++ signature/link-source mismatch in the focused fixture. The repaired assertions agree with the current `LevelScene` frame-clock and draw paths on static inspection. A runtime pass must be observed before treating it as green.

## Assertion QA

- The former low-speed false expectation is corrected: `vx=-100` stores `7.95` FPS after entry; `.010 + .024 = .034 < 1/7.95`, while `.010 + .024 + .092 = .126 > 1/7.95`, so `_019`, `_019`, `_018` follows the implementation. The final crossing margin is about 0.000214 s. The high-speed `vx=-600` row also agrees: entry uses XML 30 FPS (`.030 < 1/30`), then stored 47.7 FPS permits the next `.001` to advance once to `_018`; the following `.001` holds it. Its inline comment about residual `~.01004 plus .001` is consistent.
- The 20-frame forward and reverse loops exercise every authored path in order. `LevelScene` advances at most one frame per update, including the `.1`-second steps; the XML contains the asserted `_000.._019` and `_019.._000` sequences and all 20 PNGs exist.
- The draw classifier now rejects unknown paths, duplicate body/arm/aim draws, an arm after the body, and a mismatched arm rectangle. Initial Still arm+body and strafe body-only assertions match `LevelScene`'s arm sequence initialization and strafe selection. The aim comparison includes XY, size, and both flip bits; the baseline and strafe updates both set combat facing `-1`, so this equality is intentional.
- Raw stick threshold/sign examples match `ResolvePlayerStrafeVisual`'s strict 8689 deadzone and RX/LX sign branches. Priorities, no-sticky eligibility, non-Running spray under strafe, and sequence reset/re-entry checks are consistent with the current implementation.

## Actionable handoff / limits

1. **Build-lane owner:** after the reserved build lane releases, run the exact command below from repository root with assertions enabled. Resolve any compile or runtime failure before adding the fixture to a gate. The fixture is absent from both `port/Makefile` and `port/tests/integration.sh` today.
2. **If promoting beyond a focused smoke gate:** add an explicit positive Running footstep-spray case and an authored footstep-index assertion; the present check proves only *no* spray during strafe. Raw SDL disconnect/KeyDown behavior, Windows first-entry pose timing, impact-recovery duration, wet/sprint/slow-motion rate variants, and pixel parity remain outside this fixture.
3. **Minor robustness opportunity, not a blocker:** draw classification treats any non-arm `Game/Media/characters/player/` path as the body. Most state checks constrain the expected body, but a future extra player sprite could be misclassified. Require the unique body draw to match the active expected sequence when extending the gate. Also clear `DEATHLEAK_RENDER_HIDE_XML_IDS` in the deferred run environment if it is inherited; the fixture sets only the `ONLY` filter.

```sh
mkdir -p port/build
g++ -O2 -g -std=c++17 -Wall -Wextra -Wpedantic -UNDEBUG -Iport/compat \
  port/tests/strafe_body_cpu_test.cpp port/game/LevelScene.cpp port/game/PlayerMotion.cpp \
  Game/tinyxml/tinyxml.cpp Game/tinyxml/tinyxmlerror.cpp \
  Game/tinyxml/tinyxmlparser.cpp Game/tinyxml/tinystr.cpp \
  -o port/build/strafe_body_cpu_test
./port/build/strafe_body_cpu_test
```

Expected output on success: `PASS: strafe raw-input gate, body selection/flip, clock, priorities, spray, aim-line`.
