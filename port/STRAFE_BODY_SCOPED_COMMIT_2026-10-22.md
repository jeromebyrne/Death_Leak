# Strafe body scoped CPU commit — 2026-10-22

## Verified scope

- `LevelScene.h/.cpp`: raw-stick eligibility helper, authored StrafeForward/StrafeBack load and selection, body-facing versus aim-line-facing split, sequence-local frame-rate behavior. Current diffs in these two files are strafe-only.
- `port/tests/strafe_body_cpu_test.cpp`: CPU spy fixture covers input boundaries and non-sticky eligibility, authored frame order/flip/timing, priority overrides, positive Running footstep versus strafe suppression, and aim-line independence.
- `port/tests/integration.sh`: only its opening CPU fixture compile/run hunk belongs to this commit. Existing later route/orb/health expectation edits are unrelated and remain unstaged.
- `port/linux/main.cpp` contains the runtime call-site wiring, but is outside this task's owned files. It remains uncommitted for the main owner; this scoped commit alone is not the complete interactive strafe feature.

## Verification

- Standalone `g++ -O2 -g -std=c++17 -Wall -Wextra -Wpedantic -UNDEBUG -Iport/compat` fixture compile and run: **PASS**, `PASS: strafe raw-input gate, body selection/flip, clock, priorities, spray, aim-line`.
- `sh -n port/tests/integration.sh` and `git diff --check` on scoped source/shell files: **PASS**.
- Four warnings from pre-existing TinyXML code only; no fixture or port-source compiler warning.
- No game launch, integration-suite execution, Docker build, Xorg/GPU, or renderer/fog edit in this lane.

## Commit boundary

The commit stages the two LevelScene files, CPU fixture, this report, and only the first integration.sh hunk. All other worktree edits, including main.cpp runtime wiring and unrelated integration expectations, are preserved outside the commit.
