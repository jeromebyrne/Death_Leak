# Latest-tree controller visual gate — 0541, 2026-10-23

Scope: verification/build only; no source, fixture, shared status, GPU, Xorg, commit, or push changes.

## Frozen-tree evidence

- Inspected `RIGHT_STICK_NEUTRAL_PARITY_FIX_2026-10-22.md` and `STRAFE_AIM_LINE_FIXTURE_WATCHDOG_2026-10-22.md` before testing. No competing compiler, `make`, or `integration.sh` process was running. Source/test file SHA-256 values captured before build and all six checked unchanged after it. HEAD was `19c28af824824aa7ad90edc0f374ae5621925a06`; this gate concerns the dirty working tree, not HEAD alone.
- Focused right-stick aim-line CPU fixture, compiled with `g++ -O2 -g -std=c++17 -Wall -Wextra -Wpedantic -UNDEBUG -Iport/compat` and its LevelScene/Combat/PlayerMotion/TinyXML sources: **PASS** — `forest-1 neutral-first parity, right-stick quadrants, knife vectors, strafe hold and neutral fade`.
- Focused strafe-body CPU fixture, compiled with the integration script's LevelScene/PlayerMotion/TinyXML source list and same flags: **PASS** — `strafe raw-input gate, body selection/flip, clock, priorities, spray, aim-line`.
- The two focused builds wrote executables/logs only under `/tmp/deathleak-gate-0541`.

## Full integration gate — blocked before execution

The repository's normal `make -C port` cannot compile in this sandbox because SDL2/libpng/GL development pkg-config files are absent. Retried using the project's existing `deathleak-linux-build:bookworm` container and forced `make -C port -B -j1 all`; that also **fails to compile the current tree** with three `port/linux/main.cpp:1107` errors: `stickAim`, `aimX`, and `aimY` are not declared in that scope. They are declared at lines 694–698 inside an inner update block that closes at line 1073, while the `UpdatePlayerVisual` call is at line 1107. This is the current main-wiring blocker, independent of either passing focused CPU fixture.

Therefore `port/tests/integration.sh` was **not run**: `port/build/deathleak-linux` still dates from 22:08, before the current 22:34–22:40 edits, and running it would not validate the frozen tree. Full integration status is **not passed / blocked at build**. After the main-wiring owner fixes the variable lifetime, rebuild with the container and run integration on a newly frozen tree.

No tracked or untracked preexisting changes were altered by this verification lane, apart from this new report.
