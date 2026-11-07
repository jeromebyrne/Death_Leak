# Health projectile causal gate — adjacent commit, CPU only (2026-10-24 00:32Z)

## Verdict

The first **verified upstream combat-state divergence** relevant to the missing forest-2 hit is earlier than frame 1682: the `ee289a5` run has **two more registered combat hits** than `a4bfb61` already by frame 1559 (13 versus 11), with identical player position, HP 19, three live enemies, 49 knives fired, and the same script/assets. At frame **1614**, `ee289a5` records its first enemy kill (enemies 3→2, kills 0→1, hits 15→16), while `a4bfb61` still has three enemies, zero kills, and only 13 hits through frame 1613, and still has three/zero through frame 1617. The old run has caught up to one kill by frame 1681, but its frame-1682 3-HP projectile still connects; the new run was already one enemy down roughly 67 frames earlier and has no frame-1682 hit. This is a **real deterministic port-route behavior change** caused by the adjacent knife-lifetime change, not a flaky health endpoint. It is not yet proven that the prematurely killed enemy is the exact shooter of the old frame-1682 star; the endpoint does not expose enemy/shot identities or births.

The adjacent production diff changes player-knife `life` at creation from **0.25 s to 10 s** in `port/game/Combat.cpp`; subsequent projectile-loop expiry and swept enemy contact use that same `life` (`ee289a5` source at creation and `Combat::Update`). Its remaining production additions are `playerKnife`/`activeVisualAge` and draw fade-in; these do not move player or enemy and cannot independently account for the hit-count/kill difference in this simulate-only route. The old 0.25 s is explicitly documented in the new source as an inactive post-contact interval from original behavior, so interpreting it as active-flight TTL was a port assumption. The new longer active flight plausibly fixes that assumption and kills an enemy sooner. Whether this particular kill timing matches original Windows gameplay remains unverified; **do not revert the lifecycle solely to force the old 1-HP endpoint**.

## Bounded receipts

Used the two already-built detached binaries from the adjacent-commit gate, unchanged: `a4bfb61` at `/tmp/deathleak-health-pre-ee289a5-0023/port/build/health-first-divergence-20261024T0023Z/pre-ee289a5`; `ee289a5` at `/tmp/deathleak-health-ee289a5-0026/port/build/health-adjacent-commit-20261024T0026Z/ee289a5`. Each `--frames N` run was in `deathleak-linux-build:bookworm` with current `/srv/openclaw/workspace/death_leak_analysis/source` mounted read-only as `/src`, `SDL_VIDEODRIVER=dummy`, `SDL_RENDER_DRIVER=software`, `DEATHLEAK_TEST_ALLOW_SOFTWARE_RENDERER=1`, and `--play Game/XmlFiles/levels/forest_exploration_1.xml --script R:710,E:40,B:4500 --fixed-dt 0.0166667 --simulate-only`. `N` includes frames 0 through N-1. Only endpoint stdout was examined; no GPU/display and no shared build/edit.

| Last simulated frame | Old `a4bfb61` | New `ee289a5` | Shared player state |
|---|---|---|---|
| 1559 | hits 11, kills 0, enemies 3, shots 1 | hits 13, kills 0, enemies 3, shots 4 | `(1842.99,3109.62)`, HP 19, fired 49 |
| 1599 | hits 12, kills 0, enemies 3, shots 1 | hits 14, kills 0, enemies 3, shots 4 | `(2162.99,3108.93)`, HP 19, fired 51 |
| 1613 | hits 13, kills 0, enemies 3, shots 1 | hits 15, kills 0, enemies 3, shots 4 | `(2274.99,3108.69)`, HP 19, fired 52 |
| 1614 | not sampled | hits 16, kills 1, enemies 2, shots 3 | new `(2282.99,3108.67)`, HP 19, fired 52 |
| 1617 | hits 14, kills 0, enemies 3, shots 1 | hits 16, kills 1, enemies 2, shots 4 | `(2306.99,3108.62)`, HP 19, fired 53 |
| 1649 | hits 15, kills 0, enemies 3, shots 0 | hits 16, kills 1, enemies 2, shots 5 | `(2562.99,3108.07)`, HP 19, fired 54 |
| 1681 | hits 16, kills 1, enemies 2, shots 3 | hits 16, kills 1, enemies 2, shots 8 | `(2818.99,3107.52)`, HP 19, fired 56 |

The adjacent event logs in `port/PRIVATE_WATCHDOG_HEALTH_ADJACENT_COMMIT_GATE_20261024T0026Z.md` establish the old star's frame-1682 contact at player `(2826.99,3107.5)`, shot `(2859.04,3102.41)`→`(2850.04,3102.53)`, HP 19→16, while the new has no hit and retains HP 19. The endpoint `hits` field includes combat hits (not only player damage); this table does **not** assign a specific projectile to its extra hits. `shots` is aggregate live projectiles, not a knife-only count.

## Exact next correction gate / remaining blocker

In **isolated** adjacent worktrees, add temporary CPU trace IDs for each enemy spawn, knife birth/expiry/enemy hit, enemy shot birth, and shot hit/terrain expiry around frames 1500–1682; compare by stable enemy spawn ordinal and shot ID. Specifically determine which enemy `ee289a5` kills at frame 1614, whether the old frame-1682 star was emitted by that enemy and at what frame, and whether the new long-lived knife suppressed that shot versus merely changing another combat interaction. Only then choose an assertion: if source-correct active-flight knives explain it, update the deterministic health endpoint/encounter expectation to 4 HP for the adjacent route (while separately fixing current-tree route parity); if original Windows trace contradicts the earlier kill, correct knife collision/lifecycle semantics and rerun this exact gate. No source/test/checkpoint changes or commits were made for this investigation.
