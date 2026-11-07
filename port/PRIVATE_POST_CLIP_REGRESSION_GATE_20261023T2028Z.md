# Private post-clip CPU/current-tree regression gate — 2026-10-23

## Result: one distinct build-regression risk, not a demonstrated clip defect

**A header-only CameraShake change can leave the game binary stale under ordinary incremental `make -C port`.** `port/linux/main.cpp:11` directly includes `../game/CameraShake.h`, and the header contains inline `Start`/`Update` behavior (`port/game/CameraShake.h:14-39`). The binary target's hand-maintained prerequisite list at `port/Makefile:12` does **not** include `game/CameraShake.h`, although its compile recipe at `:14` compiles `linux/main.cpp`. A static parser confirmed direct include=true, target prerequisite=false (31 listed prerequisites). GNU make therefore has no reason to rebuild when only this header changes; stale shake behavior/API could silently survive a normal build, or a clean build could expose an API mismatch that incremental testing missed. This is a build dependency risk, not evidence that the already delivered clip used a stale binary: its capture report documents a frozen-source forced build.

## Focused verification and scope

- Current HEAD at inspection: `6dfc65b`; six tracked-dirty files were present (`CHECKPOINT_STATUS.md`, `port/COMPOSITION_DIAGNOSTIC_NOTES.md`, `port/game/ParticleSpray.h`, `port/linux/SDLRenderer.cpp`, `port/tests/integration.sh`, `port/tests/strafe_body_cpu_test.cpp`). Index had no staged paths. `git diff --check` and `sh -n port/tests/integration.sh` both exited 0.
- Compiled the current-tree `port/tests/strafe_body_cpu_test.cpp` plus its production scene/motion/TinyXML sources into a unique `/tmp` directory with `-UNDEBUG`, then ran it: **PASS** (`strafe raw-input gate, body selection/flip, clock, priorities, spray, aim-line`). Only pre-existing TinyXML warnings occurred. This checks the changed fixture/scene path, not renderer pixels or the full integration route.
- Reviewed the latest checkpoint/delivery handoffs: the 10-second active Forest clip was sent to Jerome's verified private DM; visual acceptance and matched Windows pixels remain pending. The risk here is independent of ID10/ID37/ParticleSpray visual audits and of the already diagnosed CameraShake API repair.

## Next action

Have the build owner add `game/CameraShake.h` to the application target's prerequisites (or generate compiler depfiles for the whole target), then in an isolated build verify a header-only touch triggers the compile recipe and a clean build still passes. No production/source/test/shared-status file, index, commit, GPU/Xorg process, or build output was changed in this lane; only this report was written. Existing dirty and unrelated files were preserved.
