# Current-tree loading-aware forest-2 health route (2026-10-24 00:42Z)

## Decision

A CPU-only current-tree route **does reach the same forest-2 enemy encounter** as the adjacent `ee289a5` health trace. Compensate for the forest-1 → waterfall-top loading pause with `R:726,E:40,B:4500` (instead of adjacent `R:710,E:40,B:4500`), then compare at `--frames 1699` (instead of 1683). This is an encounter/HP *route gate*, not yet an exact shot-provenance or Windows parity oracle.

## Exact command and receipt

Run from `/srv/openclaw/workspace/death_leak_analysis/source`. This uses the **existing** current-tree binary; no build, GPU, display, source edit, or shared log write. The physical current `Game/` assets are mounted read-only. The `grep` filters stdout only; renderer diagnostics go to stderr.

```sh
docker run --rm \
  -v /srv/openclaw/workspace/death_leak_analysis/source:/src:ro \
  -v /srv/openclaw/workspace/death_leak_analysis/source/port/build/health-first-divergence-20261024T0023Z/current:/probe:ro \
  -w /src -e SDL_VIDEODRIVER=dummy -e SDL_RENDER_DRIVER=software \
  -e DEATHLEAK_TEST_ALLOW_SOFTWARE_RENDERER=1 -e DEATHLEAK_TRACE_ENCOUNTERS=1 \
  deathleak-linux-build:bookworm /probe \
  --play Game/XmlFiles/levels/forest_exploration_1.xml \
  --script R:726,E:40,B:4500 --fixed-dt 0.0166667 \
  --frames 1699 --simulate-only \
  | grep -E '^(Trigger|Entered|Encounter|Simulation ended)'
```

Existing binary SHA-256: `3eec36e41b182092b44ad2d10f70571910c67e7a89f5347117303ec2956ad85d`; source `HEAD=f7d04b4`, with `port/linux/main.cpp` clean at inspection. This binary predates this lane and postdates the main.cpp timestamp; it is not a fresh rebuild against concurrent dirty renderer work.

Observed discriminating tokens:

```text
Trigger at frame 398 from Game/XmlFiles/levels/forest_exploration_1.xml player=(3120.99,3120.45) to Game/XmlFiles/levels/waterfall_top.xml
Entered Game/XmlFiles/levels/waterfall_top.xml
Trigger at frame 1074 from Game/XmlFiles/levels/waterfall_top.xml player=(3060.99,3134.72) to Game/XmlFiles/levels/forest_exploration_2.xml
Entered Game/XmlFiles/levels/forest_exploration_2.xml
Encounter trigger id=Game/XmlFiles/levels/forest_exploration_2.xml#91 elapsed_ms=-1 cooldown=0 count=2 type=kNinja
Encounter state frame=1375 level=Game/XmlFiles/levels/forest_exploration_2.xml spawned_delta=2 health_before=25 health_after=25 solid_damage=0 enemies=2
Encounter trigger id=Game/XmlFiles/levels/forest_exploration_2.xml#45 elapsed_ms=-1 cooldown=0 count=1 type=kGhost
Encounter state frame=1424 level=Game/XmlFiles/levels/forest_exploration_2.xml spawned_delta=1 health_before=25 health_after=25 solid_damage=0 enemies=3
Encounter state frame=1479 level=Game/XmlFiles/levels/forest_exploration_2.xml spawned_delta=0 health_before=25 health_after=22 solid_damage=0 enemies=3
Encounter state frame=1574 level=Game/XmlFiles/levels/forest_exploration_2.xml spawned_delta=0 health_before=22 health_after=19 solid_damage=0 enemies=3
Simulation ended at Game/XmlFiles/levels/forest_exploration_2.xml player=(2826.99,3107.5) orbs=36 keys=0 health=19 focus=100 balance=36 spawned=3 enemies=2 hits=16 kills=1 fired=56 shots=8 grounded=1
```

Two `Encounter projectile hit` records immediately precede the 1479 and 1574 state lines, each `damage=3`, `health_before=25` and `22` respectively. Full endpoint includes nondeterministic `elapsed`; do not assert that field.

## Loading compensation and match scope

`port/linux/main.cpp:1423-1428` enters `LoadingPhase::FadeIn` on forest-1 → waterfall-top; `:703-705` pauses gameplay during `FadeIn` and `OpaqueHold`, and `:827-861` loads the scene after a 0.20-s fade then exits a minimum 0.05-s hold. `FadeOut` resumes gameplay (`:866-873`). At this fixed dt the observed route recovers by adding **16 scripted R slots** and 16 total frames. The unadjusted route's frame-398 boundary is documented in `PRIVATE_WATCHDOG_HEALTH_ROUTE_ORACLE_20261024T0040Z.md`.

The adjacent `ee289a5` log `/tmp/deathleak-provenance-new-0036.log` enters forest 2 at 1058 and reaches the same trigger IDs `#91` (2 ninjas) at 1359 and `#45` (ghost) at 1408. Current compensated frames are exactly +16 for those three landmarks (1074, 1375, 1424). Its frame-1698 endpoint matches adjacent frame-1682 position `(2826.99,3107.5)`, HP 19, spawned 3, kills 1, fired 56, shots 8, and hits 16. It does **not** match collected orbs (current 36 versus adjacent 35), and the two player-hit frames are +18/+19, not exactly +16 (1479/1574 versus 1461/1555). Therefore this verifies encounter identity and useful health parity, not byte-identical combat causality. A trial `R:727,E:40,B:4500 --frames 1700` also reached the same encounter and HP 19 but ended 8 x-units farther at `(2834.99,3107.49)`; `R:726` is the tighter positional gate.

## Next source-backed gate

After the concurrent GPU worker's build activity is clear, use an isolated instrumented worktree/binary (not shared source/tests) to trace **current-tree** shot spawn, knife lifetime/end, enemy hit/death, and player-hit ordinals on this `R:726,E:40,B:4500` route. Align by forest-2 entry/encounter-relative frames and compare to the adjacent `ee289a5` `PROBE` oracle in `PRIVATE_WATCHDOG_HEALTH_ROUTE_ORACLE_20261024T0040Z.md`; explicitly reconcile the +2/+3 hit-frame offset and one-orb difference before promoting a current-tree exact HP expectation. The source's active knife lifetime bound is still a surrogate for Windows' inactivity behavior, so this cannot establish Windows parity. No existing `integration.sh` expectation should change from this gate alone.

This report is this lane's sole write. No build, GPU/display run, shared source/test/status/index change, or Waterfall packet work was performed.
