# Independent QA: Running rate order (CPU/source-only)

Reviewed at HEAD `a4fc2e3` against the current dirty tree. No game, build, Docker, GPU, or test execution was performed in this lane.

## Verdict

**Scoped order change is source-correct; integration is not yet complete.** Windows `Character::UpdateAnimations` selects Running, calls `AnimationPart::AnimateLooped()` using the sequence's stored `Framerate()`, then calls `SetFrameRate` from velocity (`Game/src/GameObjects/Character.cpp:1006-1053`; `Game/src/Animation/AnimationPart.cpp:159-196`). `SetSequence` calls `Restart` but does not reset the sequence's rate. The port's changed `LevelScene.cpp:1253-1275` now advances Running at `currentRate`, and stores the velocity-derived rate afterward; initial `currentRate = rate` is loaded at `LevelScene.cpp:419-423`. It retains the existing strafe ordering, one-frame-per-update rule and re-entry state. `git diff --check -- port/game/LevelScene.cpp` passed.

The order patch is bounded, not a claim of complete Windows rate parity: original default `mRunAnimFramerateMultiplier` is `3.0f` and has sprint, water and slow-motion branches (`Character.cpp:49-56,1035-1052`), whereas this port calculation remains the pre-existing `1.8 * 2.65 * abs(vx/60)` clamp. That pre-existing conversion was not changed or validated by this patch.

## Fixture discrimination

`port/tests/running_rate_order_cpu_test_RUNNING_RATE_ORDER_20261023T2136Z.cpp` loads the real Forest-1 XML into `LevelScene`, spies texture paths from `Draw`, and checks the Running body. Its second tick (`vx=60`, prior delta `.008`, expected frame 001) distinguishes stored-60-first from the previous immediate-4.77 behavior. Later `.100/.105` ticks distinguish continued low-rate use and fractional carry; `.500/.001` check one-step-per-update with excess carry. Still/re-entry checks rate persistence while the cursor resets. Deliberately unequal `dt=.003` and prior deltas exercise the existing prior-rendered-delta input. These are meaningful positive draw-command assertions, not a same-state Windows visual/pixel oracle.

**Test guard:** all calls to `tick()` are inside `assert(...)`; compiling with `NDEBUG` would erase the stimulus and yield a false PASS. The prior scoped test report says it was compiled with `-UNDEBUG`, but this QA lane did not rerun it. Any permanent runner must retain `-UNDEBUG` (or convert to non-elidable checks). The fixture is currently untracked and has no reference in `port/tests/integration.sh`, so it is not yet a durable integration gate.

## Handoff / readiness

The `LevelScene.cpp` hunk is narrowly reviewable and ready for coordinator staging after ownership release. Include the unique fixture and explicit CPU-test invocation in the chosen gate if this behavior must remain protected; compile that fixture with assertions enabled and run from repository root. Do not stage the whole dirty tree: `integration.sh`, `SDLRenderer.cpp`, `strafe_body_cpu_test.cpp`, checkpoint files, and numerous reports have concurrent unrelated changes. No commit/push or shared-file edit was made here.
