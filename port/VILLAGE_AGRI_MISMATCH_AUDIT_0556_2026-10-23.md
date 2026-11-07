# Village-agriculture integration mismatch — 05:56, 2026-10-23

Scope: CPU/read-only inspection of the frozen-tree integration trace, current fixture/source, and saved route artifacts; this report is the only write. No build, executable/integration/game run, GPU/display use, source/test/shared-status edit, commit, or push. The worktree contains unrelated concurrent changes.

## Verdict

The three-value `village_agri` assertion is stale against the observed current route, not evidence of a newly introduced aim-line or agriculture gameplay regression. The same forest-start script reaches both asserted transition frames (shrine 5785, agriculture 8206) and the exact asserted endpoint `(2711.18,-1286.6)`. Its actual endpoint is `orbs=188 health=4 balance=188`, alive, with 10 spawned, 3 kills, 471 fired (`port/build/aim-line-main-scope-fix-2026-10-23/integration-trace.log:7524,7575`). The first failure is precisely the old 187/1/187 grep at `port/tests/integration.sh:602`; the save checks immediately after it did not execute in this run. The earlier continuous village assertion on the same route prefix has already been refreshed to 100/4/100 in this dirty fixture.

## Independent save/causal checks

- Current `port/build/extended-village-save.txt` starts with the expected agriculture level/position, saved health 4, and `B:188`. It has exactly 188 distinct persistent level/XML orb IDs: forest 1=7, waterfall top=18, forest 2=11, barn=8, forts=20, central=55, shrine=37, agriculture=32. The orb count equals balance; the excess is not a balance-only mutation or duplicate ID.
- The preserved **same-script, same-endpoint** `port/build/extended-village-save.pre-2316-2026-10-22.txt` records 187 distinct IDs, health 1, `B:187`; its level counts match the current save except waterfall top=17. Set difference is exactly one newly collected authored object, `Game/XmlFiles/levels/waterfall_top.xml#88`, with no old-only ID. The XML contains one `<CurrencyOrb id="88">` at `(259.185242,2974)`. This is stronger localization than a +1 endpoint inference. The snapshot predates several changes (mtime 08:33 versus current 22:55 on Oct 22), so it does **not** by itself attribute the capture to one commit.
- The source mechanism is concrete: `LevelScene.cpp:774-833` advances authored small-orb hover/tracking; `main.cpp:690-694` invokes that update before collection, also in simulate-only mode; `LevelScene.cpp:622-636` collects inside 100 units and persists the authored ID. Commit `7fd5642` introduced the waterfall-top motion path. This is consistent with #88 being newly reachable along the same route; no frame-exact pickup trace was captured, so the particular update tick is not proven.
- Health 4 already appears at the earlier village-central prefix on the current route (`continuous-village-save.txt` and the passing current assertion at integration.sh:167), and remains 4 at agriculture. The historical same-script agriculture endpoint had health 1, but that older artifact is not a controlled before/after combat trace. Commit `ee289a5` changed active knife lifetime, plausibly altering encounter outcomes; the old versus current endpoint also differs in total hits (53→57) and remaining shots (1→2), while spawned/kills/fired agree. No per-hit `DEATHLEAK_TRACE_ENCOUNTERS` timeline exists here. Therefore an exact avoided damage event and original-game combat parity remain unproven. There is no evidence that the aim-line visual scope fix caused the health delta; `stickAim` is disabled for scripted/simulate-only play (`main.cpp:699-706`).

## Smallest safe patch for the integration owner

Only after taking the exclusive test lane, change the three expectations in this agriculture block of `port/tests/integration.sh`:

```diff
-... 'village_agri.xml player=(2711.18,-1286.6) orbs=187 .*health=1 .*balance=187'
+... 'village_agri.xml player=(2711.18,-1286.6) orbs=188 .*health=4 .*balance=188'
-assert len(orb_ids) == len(set(orb_ids)) == 187, ...
-assert 'B:187' in lines
+assert len(orb_ids) == len(set(orb_ids)) == 188, ...
+assert 'B:188' in lines
```

Retain exact trigger frames/position, unique-ID, balance-equals-earned-orbs, and no-death checks. Do not alter gameplay source or mechanically update later agriculture-wall/return assertions: the suite stopped at this grep, so those must be judged from the next fresh run and its own saves. This audit does not claim a full integration pass.
