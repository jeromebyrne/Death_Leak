# Fresh HEAD integration rerun — 2026-10-22

Owner lane: exclusive Docker build/integration; no physical GPU capture or display use. Source HEAD before test patch: `c1725ee`. Unrelated working-tree changes were untouched; `PORT_WORK_STATUS.md` and `CHECKPOINT_STATUS.md` were not edited.

## Verified test-only updates

- Forced a fresh Docker build (`make -B -C port`) on HEAD. `sh -n port/tests/integration.sh` passed.
- Fresh focused arena simulation returned `spawned=2 enemies=1 hits=14 kills=1`, confirming the review's stale `2/2/6/0` grep. Updated only that grep, with a comment linking it to source-correct active knife flight. The first full-suite rerun passed this grep and the following resume/cooldown/expired-trigger assertions.
- First rerun then hit an environmental write error: ignored `port/build/expired-encounter-save.txt` was root-owned (0:0, mode 644). Preserved it by moving it to `port/build/expired-encounter-save.pre-head-rerun-2026-10-22.txt`, allowing the suite's same-path output to be recreated by UID 1001. This is not a source/test assertion failure.
- Next rerun stopped at the east-route orb assertion (`integration.sh:144`): actual `underwater_cave.xml ... orbs=17 ... balance=17`, expected 16/16. This route does not fire. Commit `7fd5642` added waterfall-top authored small-orb hover/tracking and collection mechanics after the original test baseline. Updated only this source-backed grep to 17/17; the subsequent rerun passed it.

## Remaining first failure

The most recent full-suite rerun reached the uninterrupted `R:710,E:40,B:4500` village route and stopped at `integration.sh:157` (line numbers after the two comments). Exact current output: `village_central.xml player=(7287.18,-1525.13) orbs=100 keys=0 health=4 focus=100 balance=100 spawned=10 enemies=1 hits=57 kills=3 fired=266 shots=6`. Expected grep: same position, `orbs=99 ... health=1 ... balance=99`. The route's saved file contains 100 distinct orb IDs and `B:100`. One extra orb is consistent with the waterfall-top tracking change; different health and combat counts are consistent with the corrected longer knife flight, but exact causation of the health delta has not been isolated. This is an observed downstream assertion failure, not a complete suite pass. No speculative route, save, or later assertion edits were made.

Trace: ignored `port/build/integration-head-rerun-2026-10-22.log` (most recent `sh -x` run). The three full-suite attempts stopped respectively at the root-owned artifact, east-route 16/16 grep, and village 99/1/99 grep. Current test patch is limited to the two individually verified assertions above. The entire script beyond the village grep remains unexercised on this HEAD rerun, so no final integration PASS is claimed.

## Handoff

Next owner should isolate the continuous-route 100-orb/health-4 result against the pre-orb-tracking and pre-knife-lifetime behavior, then update only proven source-correct expectations (including the adjacent unique-ID/`B:` save checks if appropriate). Re-run the full suite to expose subsequent route regressions. Keep the Docker build/test lane exclusive and continue avoiding GPU/display capture in this lane.

Verified test-only changes were committed as `814fbd3` and pushed to `origin/feature/linux-renderer-foundation`. The report itself remains an untracked handoff file, per the commit scope restriction.
