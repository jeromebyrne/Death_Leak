# Strafe CPU fixture / aim-line watchdog — 2026-10-22

Scope: `port/tests/strafe_body_cpu_test.cpp` only, plus this report. No GPU/Xorg use; no shared source, integration script, or right-stick fixture edits.

## Source-backed reconciliation

`Game/src/GameObjects/Player.cpp` refreshes the aim-line hold while strafing (`mIsStrafing`) and displays it at full alpha during the 0.05-second hold. The current dirty `port/game/LevelScene.cpp` similarly refreshes on `strafe.active` at lines 1079–1085. Thus the forest spy's first StrafeBack update draws **body then aim line**, with no authored StrafeBack arm. The old one-draw assertion at line 119 was obsolete. The same applies to the low-speed StrafeBack sequence later in the fixture.

Updated those two expectations to require exactly two draws, identify the aim texture, and assert full alpha. Existing body-frame/clock, flip, priority, spray, and later aim-line checks remain intact.

## Focused verification

- Before each build, checked for competing `cmake`, `ninja`, C++ compiler, or `integration.sh` process; none was active.
- Strafe fixture: compiled with the integration script's `g++ -O2 -g -std=c++17 -Wall -Wextra -Wpedantic -UNDEBUG -Iport/compat` source list; executed `env -u DEATHLEAK_RENDER_HIDE_XML_IDS /tmp/strafe-body-cpu-test-watchdog`. **PASS**: `strafe raw-input gate, body selection/flip, clock, priorities, spray, aim-line`.
- Right-stick fixture (owned by another worker): compiled with its report's C++ source list plus `Combat.cpp`; executed `env -u DEATHLEAK_RENDER_HIDE_XML_IDS /tmp/right-stick-aim-line-cpu-test-watchdog`. **PASS**: `forest-1 neutral-first parity, right-stick quadrants, knife vectors, strafe hold and neutral fade`.
- `git diff --check -- port/tests/strafe_body_cpu_test.cpp`: **PASS**. Compiler emitted only pre-existing TinyXML warnings.

## Remaining gate

The prior integration rerun8 blocker at strafe line 119 is resolved **against the current dirty source tree**. No latest-source full integration run/rebuilt binary was performed; the next owner must run it after the concurrent LevelScene/main/right-stick fixture work settles and adjudicate any later blocker. The right-stick fixture is still untracked in this tree and is outside this lane.

No commit or push: the corrected fixture depends on concurrent, uncommitted LevelScene aim-line changes. A fixture-only commit against current `HEAD` would encode an expectation that the committed source does not yet satisfy. Preserve this scoped diff and commit it together with the verified source integration once ownership is released.
