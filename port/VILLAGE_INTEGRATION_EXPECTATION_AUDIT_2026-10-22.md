# Continuous village integration expectation audit — 2026-10-22

Scope: strictly read-only source/artifact audit, except this report. No binary, build, Docker, integration suite, GPU, or display run; no shared file edits. Audited checkout HEAD `814fbd3`, the fresh rerun report/trace, current saved artifact, Linux port source, authored XML, and relevant commit diffs. The rerun owner retains the exclusive execution lane.

## Finding and exact first test-only correction

The first failing assertion in `port/tests/integration.sh:157` is stale against the fresh HEAD run. `port/build/integration-head-rerun-2026-10-22.log:640-717` repeatedly records the same uninterrupted `R:710,E:40,B:4500` output, reaching the expected trigger frames 398, 1058, 1714, 2714, and 4037 and the same final position `(7287.18,-1525.13)` in village central. The final line is `orbs=100 keys=0 health=4 focus=100 balance=100 spawned=10 enemies=1 hits=57 kills=3 fired=266 shots=6 grounded=1`. The old grep expects 99/health 1/balance 99. The saved `port/build/continuous-village-save.txt` has `B:100`, saved health 4, **100 distinct** `Game/XmlFiles/levels/<level>.xml#<id>` entries, and no duplicate; every entry resolves to exactly one `<CurrencyOrb>` in its authored XML. The count partitions as forest 1=7, waterfall top=18, forest 2=11, barn=8, forts=20, central=36. Thus the observed 100 is earned pickup persistence, not an injected balance, duplicate ID, or save-format mismatch.

Minimal immediately supportable assertion update, **only in this village block**:

```diff
-... 'village_central.xml player=(7287.18,-1525.13) orbs=99 .*health=1 .*balance=99'
+... 'village_central.xml player=(7287.18,-1525.13) orbs=100 .*health=4 .*balance=100'
-assert len(orb_ids) == len(set(orb_ids)) == 99, ...
-assert 'B:99' in lines
+assert len(orb_ids) == len(set(orb_ids)) == 100, ...
+assert 'B:100' in lines
```

Preserve the existing uniqueness check, trigger/position checks, and positive-survival signal. Do not change game logic or route input on this evidence.

## Source-backed cause and confidence boundary

`7fd5642` changed gameplay pickup geometry, not merely rendering: `port/game/LevelScene.cpp:455-470,683-730` now advances small waterfall-top authored orbs in simulation through sine hover, sticky `<130` proximity tracking, and motion; `:581-592` collects at current moved XY under a strict `<100` check. `port/linux/main.cpp:685-698` invokes motion **before** collection even with `--simulate-only`; each collected authored orb enters the persistent level-path/XML-ID set and increments `orbBalance` once (`LevelScene.cpp:488-489,581-596`, `main.cpp:1229-1234`). The separate east-route assertion was already directly observed to rise 16→17 on HEAD. This is a concrete source path for the village route's one extra earned orb, with 18 waterfall-top IDs in the current village save. **The precise newly captured XML ID is not established**: no pre-`7fd5642` snapshot of this same route's ID set was available for a set-difference comparison. Do not name one as proven.

`ee289a5` is the concrete combat behavior change since the old route expectations: `port/game/Combat.cpp:228-240` replaces the erroneous 0.25-second active knife lifetime with a 10-second offscreen collection bound; `:382-456` keeps knives until collision/lifetime and now allows farther enemy hits/kills. Held `B` means right+fire (`port/linux/main.cpp:700-706`). The fresh focused arena run already proved this effect there (`spawned=2 enemies=1 hits=14 kills=1`), and the village run reports 10 spawned/3 kills with health 4. A surviving extra 3 health is **consistent with**, but does not prove, one fewer 3-damage ninja projectile hit: `Combat.cpp:335-348` assigns ninja-star damage 3 and `:429-442` applies hits; other damage/encounter timing also exists. The rerun trace did not enable `DEATHLEAK_TRACE_ENCOUNTERS`, has no per-hit timeline, and no same-route pre-knife health trace. Therefore the exact causal chain for health 1→4 is not isolated by this read-only audit. It is not justified to call the 4 a gameplay regression or to claim source-perfect combat parity solely from the endpoint. It **is** justified to refresh this deterministic observed HEAD assertion while retaining survival and later route checks; a lane owner seeking strict causal proof can compare the same route with targeted encounter trace or pre-change binary, without changing the source here.

## Downstream assertion exposure; no blanket +1 edits

`integration.sh:165-368` contains chained continuous routes with 227/290/296/306/327/343/375/383/388 endpoint-orb and `B:`/unique-ID checks, plus health 1, exact events, `kDeflection` balance/debit, and subset/difference invariants. The script stops at line 157, so **none of these were exercised in the fresh HEAD full-suite rerun**. The waterfall change could add an ID on a shared prefix, while longer-lived knives can alter kills, damage, survival, and potentially pickup paths. The later 375-orb purchase is especially sensitive: a +1 there can change a real debit/balance premise, not just a grep. Do not mechanically add one to every total or rewrite health 1 everywhere. Resume the suite after the three village expectation edits and adjudicate each *first observed* downstream mismatch from its own current output and saved unique-ID set; preserve set inclusion/difference, purchase debit, survival, and trigger assertions. The same caution applies to later continuous full-ending/death-route assertions beyond this block.

Evidence limits: all values above are from an existing rerun artifact, saved file, or static source inspection. This audit does not claim a complete integration PASS, an exact newly collected waterfall ID, or frame-exact attribution of the health delta.
