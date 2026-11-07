# Knife projectile rotation — ownership blocker (2026-10-23)

## Source finding

- Windows `Game/src/GameObjects/Player.cpp:452-486` creates player knives without enabling `mSpinningMovement`. `Projectile.h:128` defaults `mRotateToDirection=true`; `Projectile.cpp:469-481,801-834` updates their angle every active flight frame from **current velocity**. Gravity changes that velocity during flight. Thus player knives pitch during flight, but do not do continuous 20-rad/s tumbles in the original Windows source.
- Windows ninja stars explicitly call `SetSpinningMovement(true)` in `NPC.cpp:326`; their active-flight angle subtracts `20*delta` radians in `Projectile.cpp:473-481`. This is a different projectile contract. The reported desire for player-knife *spin* would be an intentional behavioral change, not literal source parity.
- Linux `port/game/Combat.cpp` initially submitted player shots with `sprite.flipX` only and no `rotationDegrees`, so even source velocity-following pitch was absent. A separate concurrent worker edited this same draw block while this investigation was underway, attempting a velocity-derived angle. The transient overlapping diff combined two incompatible orientation paths.

## Work and verification

- I prepared an active-flight `20*dt` spin state and standalone CPU test, which passed locally at 60 Hz for angle progression and both facing transforms. I also started an isolated Docker build output under `/tmp/deathleak-knife-build-20261023` because the sandbox lacks SDL development packages.
- During diff review the same `Combat.cpp` knife draw block changed concurrently. I stopped the build and removed **only my own** `Combat.cpp`, `Combat.h`, `Makefile`, helper, and test changes. Targeted `git status --short` and `git diff --check` for those paths were empty afterward. The isolated build did not complete and no compiled integration verification is claimed.
- No production fix, tracked test, commit, push, shared build output, status file, or physical GPU capture was delivered by this worker. This report is the sole retained artifact.

## Integration decision needed

Assign one owner to the knife draw/update block. For strict original behavior, compute each active player's knife orientation from its current gravity-adjusted velocity each frame, preserving Windows leftward reflection and SDL Y conventions; ensure a CPU test observes different submitted angles at successive flight times, not just different launch aim. If Jerome explicitly wants visible full tumbling of player knives, apply a continuous angular state as a deliberate deviation from Windows and test that separately. In either case a fresh physical-GPU clip showing several frames of the same knife in flight is still required for visual acceptance; CPU tests alone cannot establish that the sprite appears to rotate on the real renderer.
