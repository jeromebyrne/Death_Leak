# Forest-2 health shot provenance — bounded adjacent CPU trace (2026-10-24 00:36Z)

## Verdict

**Yes: the earlier kill suppresses the exact old frame-1682 star.** Stable spawn ordinal **enemy 1**, a `kNinja` spawned at frame 1359, is the old frame-1682 shot's shooter. In `a4bfb61`, enemy 1 fires stable shot **60** at frame **1668**, is killed at **1674**, and that already-fired shot hits the player at **1682** for 3 HP (19→16). In `ee289a5`, the same enemy is killed at **1614**, **54 frames before its old shot birth**, so it cannot emit that shot. The new run's shot 60 is instead from enemy 2 at frame 1674 and does not hit by frame 1682. This resolves the exact shooter/suppression uncertainty in the 00:32Z causal gate, not merely a correlation between total kills and health.

The extra damage to enemy 1 is traceable to the adjacent knife lifetime delta: knives 47/48, born at frames 1489/1503, expire at frames 1503/1517 in the old 0.25-s build, but in the new 10-s build hit enemy 1 at **1510/1519** (1.5 damage each). Their two additional contacts move enemy 1's lethal knife-56 hit from old **1674** to new **1614**. Common projectile hit events at frames 1461 and 1555 and common player position/HP through frame 1681 remain consistent with the prior reports. This was an instrumentation-only comparison of adjacent commits; it does not claim that the entire later route matches original Windows gameplay.

## Reproducible trace receipts

- Isolated detached worktrees: `/tmp/deathleak-health-pre-ee289a5-0023` at `a4bfb61` and `/tmp/deathleak-health-ee289a5-0026` at `ee289a5`. Added temporary `probeId`/`probeSource` fields and stdout event probes **only in those detached worktrees**, then rebuilt `port/build/deathleak-linux` in `deathleak-linux-build:bookworm`. No shared source, display, GPU, integration test, or checkpoint was touched.
- Each instrumented binary ran with the same current `source/` assets mounted read-only, `SDL_VIDEODRIVER=dummy`, `SDL_RENDER_DRIVER=software`, `DEATHLEAK_TEST_ALLOW_SOFTWARE_RENDERER=1`, `DEATHLEAK_TRACE_ENCOUNTERS=1`, `--play Game/XmlFiles/levels/forest_exploration_1.xml --script R:710,E:40,B:4500 --fixed-dt 0.0166667 --frames 1683 --simulate-only`. Probe `f` matches the built-in encounter frame numbers (including built-in frame-1682 hit). The original ordered encounter spawns are enemy 1 ninja `(392.986,3413.06)` and enemy 2 ninja `(92.9863,3473.06)` at frame 1359, then enemy 3 ghost at 1408.
- Logs: old `/tmp/deathleak-health-pre-ee289a5-0023/port/build/provenance-0036.log` SHA256 `f140e586424a528cfad69b126d7d2703d266925821b5bc052cb8f1285306b17a`; new `/tmp/deathleak-provenance-new-0036.log` SHA256 `5a9dea9ce4d570093bfe482939569df9e630a55250cfc99fc1a370d96d09c16b`.

| Event | Old `a4bfb61` | New `ee289a5` |
|---|---|---|
| Knife 47 born 1489; knife 48 born 1503 | 0.25-s `life`; shot 47 expires 1503, shot 48 expires 1517 | 10-s `life`; shot 47 hits enemy 1 at 1510 (HP 11→9.5); shot 48 hits enemy 1 at 1519 (9.5→8) |
| Enemy 1 after shared knife 56 at 1614 | HP 3.5→2; remains alive | HP 0.5→0; killed at 1614 |
| Would-be damaging star | Enemy 1 emits shot 60 at 1668, `(2984.99,3099.72)`, velocity `(-539.761,16.053)` | Enemy 1 absent; **no** frame-1668 shot from it |
| Later enemy-1 death / other shot | Knife 59 kills enemy 1 at 1674, after its shot was already in flight; enemy 2 emits shot 61 at 1674 | Enemy 2 emits shot 60 at 1674; no player hit through 1682 |
| Frame 1682 endpoint | Shot 60 from enemy 1 travels `(2859.04,3102.41)`→`(2850.04,3102.53)`, hits player `(2826.99,3107.5)`, HP 19→16 | HP 19, no player hit, no enemy-1 shot |

`Game/src/GameObjects/Projectile.cpp` constructor stores the player's `0.25f` final argument as `m_maxTimeInActive` and starts `m_isActive(true)` (lines 25–43); its update advances active projectiles without that timer (lines 425–499) and applies the timer only after inactivity (lines 502–520). `Game/src/GameObjects/Player.cpp` passes that argument at its projectile construction (around line 452). Thus the old port's 0.25-s **active** TTL is not the source contract. The new 10-s active bound is a port cleanup surrogate, not a proven Windows exact despawn timer.

## Precise next correction gate

1. Preserve the adjacent active-flight behavior; **do not restore old 0.25-s active expiry to recover the 1-HP expectation**. Add a narrow CPU regression oracle for this route asserting knife 47/48 reach enemy 1, enemy 1 dies by frame 1614, no enemy-1 shot is born at 1668, and HP is 19 (not 16) at frame 1682. If keeping only public aggregate assertions, first validate this provenance in a private fixture and set the adjacent-route final health expectation to **4 HP rather than 1 HP** (the later 4-HP result is from the existing 00:26Z full-route receipt, not this 1683-frame run).
2. Separately reconcile the dirty/current `f7d04b4` route's earlier loading-frame-398 divergence before changing its integration endpoint. This adjacent-comparison proof does **not** establish a current-tree final assertion, nor a matched Windows encounter trace. Any claim of original-runtime exact kill timing still needs a Windows enemy/shot capture; source inspection only establishes that 0.25 s was not an active TTL.

No commit was made.
