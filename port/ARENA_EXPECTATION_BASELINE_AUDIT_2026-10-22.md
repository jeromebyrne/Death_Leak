# Arena integration expectation baseline audit — 2026-10-22

Scope: read-only CPU/static investigation at HEAD `c1725ee`; no integration, Docker, game, GPU, build, or source edit was performed. The only write is this report. Existing unrelated working-tree changes were left intact.

## Exact failure

`port/build/all_flip_integration_trace.log` (mtime 2026-10-22 17:01:41 PDT) reaches the arena `combat` command and then exits under `set -eu` at `port/tests/integration.sh:70`:

```sh
combat=$($bin --play Game/XmlFiles/levels/first_ninja_arena.xml --start-encounter 0 --script F:300 --fixed-dt 0.0166667 --simulate-only --save-file port/build/integration-save.txt)
printf '%s\n' "$combat" | grep -q 'spawned=2 enemies=2 hits=6 kills=0'
```

Actual `Simulation ended` line: `spawned=2 enemies=1 hits=14 kills=1 fired=18 shots=3`, with player `(5167.17,-1798.36)`, health 25, focus 100. Thus the first failing assertion is the stale combined combat-count grep, specifically `enemies=2 hits=6 kills=0` versus `enemies=1 hits=14 kills=1`; spawn count still matches. The trace does not reach the resume assertions or suite PASS.

## Baseline and cause

- The exact expectation was introduced in `5174a8c` (Oct 19) and remains unchanged at HEAD (`git blame -L 69,72 port/tests/integration.sh`). A prior full suite did pass: `port/build/physical-menu-2026-10-22/integration-final.log` ends in the suite PASS; its file predates the later knife change.
- Commit `ee289a5` (Oct 22 14:33 PDT) changed the **player knife active lifetime** in `port/game/Combat.cpp` from 0.25 s to a 10 s offscreen collection bound, leaving on-contact removal. This is the source-backed plausible reason more stationary-fired knives reach the arena NPC over 300 frames, raising hits and yielding a kill. `Combat::Update` increments hits through `DamageEnemy`, removes a knife on contact, and erases dead enemies. The original Windows `Game/src/GameObjects/Projectile.cpp` sets `m_maxTimeInActive = 0.25f` only after making the projectile inactive; it is not an active-flight time-to-live. Reverting to the old 0.25 s to satisfy the grep would restore the known source mismatch.
- The failing trace predates `c1725ee` (commit time 17:05:52 PDT; trace 17:01:41). HEAD's `LevelScene.cpp` delta only changes ordinary sprite draw rotation; its other changed files are renderer geometry/tests/Makefile. Hence this failure is **pre-existing relative to HEAD** and not attributable to its all-flip rotation change. The precise first commit that changed arena counts was not bisected; `ee289a5` is the direct, source-supported causal candidate. The trace proves the failure for the pre-HEAD binary, not an independently executed HEAD suite.

## Minimal next action for integration owner

Update only the arena `combat` assertion at `port/tests/integration.sh:70` to the observed deterministic count (`spawned=2 enemies=1 hits=14 kills=1`), with a brief comment tying it to the source-corrected active knife lifetime. Then run the full integration once under the build/test lane owner's coordination, checking all subsequent assertions and recording the result. If counts differ on a freshly built HEAD binary, investigate that delta before changing expectations again. Do not change gameplay or revert the knife lifetime solely to make this outdated grep pass.
