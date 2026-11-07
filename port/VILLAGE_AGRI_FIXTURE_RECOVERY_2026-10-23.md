# Village-agriculture integration fixture recovery — 2026-10-23

Scope: exclusive Docker integration CPU test lane. Edited only `port/tests/integration.sh` and this report. Did not edit gameplay/render source, shared checkpoints, or unrelated dirty work; no commit or push. The full integration runner uses its existing offscreen Mesa llvmpipe test mode; no physical GPU/Xorg capture was used.

## Evidence-backed fixture changes

- The audited continuous forest-to-agriculture endpoint now expects `(2711.18,-1286.6)`, `orbs=188`, `health=4`, `balance=188`, with shrine/agriculture trigger frames 5785/8206 unchanged. The saved file has 188 distinct authored orb IDs and `B:188`, including `waterfall_top.xml#88`; no-death and uniqueness gates remain.
- The next fresh full run exposed the agriculture east-wall expectation: same position `(3519.8,-1288.5)` and death, but death at frame 8953 rather than 8952, with `orbs=192`, `balance=192` rather than 191. Refreshed only those observed values, retaining exact death/position gates. This longer route shares the audited +1 waterfall pickup; the extra survival frame is consistent with the independently observed four-health route prefix, but no frame-by-frame damage attribution is claimed.
- The next full run exposed the westward return endpoint: all seven asserted transitions still occurred at their specified frames (9527/11943/13701/15025/16010/16651/17306), and `(−1078.02,3110.97)`, `orbs=190`, `balance=190` remained unchanged. Refreshed only `health=1` to observed `health=4`. The return save has 190 distinct IDs and `B:190`.

## Full integration execution and next blocker

Three full Docker `sh -x port/tests/integration.sh` runs, using `deathleak-linux-build:bookworm`, each advanced past the preceding failure. Logs (under `port/build/`): `village-agri-fixture-recovery-integration.log`, `village-agri-fixture-recovery-rerun2.log`, `village-agri-fixture-recovery-rerun3.log`. **The suite is not passing.** Rerun 3 reached `port/tests/integration.sh:775` and failed the independent `range_kill` encounter assertion. Actual `encounter_roll.xml --script B:330` endpoint: `spawned=1 enemies=1 hits=14 kills=0 fired=20 shots=8`, player health 22. The assertion requires `spawned=1 enemies=0 ... kills=1`. No encounter expectation was changed: this is a potential combat behavior regression, not a stale agriculture count/health endpoint, and requires source/fixture causality investigation by its owner. No later assertions were exercised by this run.

`git diff --check -- port/tests/integration.sh` passed. The unchanged main/LevelScene source hashes match the frozen aim-line scope-fix report (`ffcae062`, `cda0ed58`, `16604406` respectively). The existing mixed dirty tree was preserved; do not treat this as a passing or ready-to-commit integration gate.
