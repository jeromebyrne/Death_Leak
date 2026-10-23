# Village integration gate rerun9 — 2026-10-22

Status: in progress. Exclusive non-GPU Docker build/test lane reserved. This report is a durable checkpoint; no suite pass is claimed.

## Initial evidence

At 2026-10-23 00:53 UTC, Git HEAD was `bbc113b`, `port/tests/integration.sh` already contained unrelated uncommitted village expectation edits, and no competing Docker build/integration process was visible. The latest completed trace was rerun8, which stopped at the earned-deflect endpoint grep: actual `village_agri.xml player=(1951.18,-1278.17) orbs=383 keys=0 health=4 balance=8 spawned=18` with `Deflection unlocked=1 count=1`, versus the assertion's `health=1`. The saved file held 383 unique IDs, `B:8`, `U:kDeflection`, and shrine NPC #189. The binary was rebuilt after rerun8, so that evidence alone is not a latest-binary pass.

## Action and next checkpoint

Change only the earned-deflect endpoint health expectation from 1 to 4; retain all other invariants. Run shell syntax, a fresh established Docker build, and the full integration script; report the first actual failure and process exit.

## Build checkpoint

`sh -n port/tests/integration.sh` and `git diff --check -- port/tests/integration.sh` passed. Fresh Docker `make -B -C port -j2` exited 0; log: `port/build/village-integration-gate-rerun9-build-2026-10-22.log` (only existing TinyXML compiler warnings). Full integration now running in the same Docker image, offscreen/test-only renderer; no physical GPU or capture.

## Rerun9 outcome

Full `sh -x port/tests/integration.sh` in `deathleak-linux-build:bookworm` exited **1**. Trace: `port/build/village-integration-gate-rerun9-2026-10-22.log`. This is **not** a suite pass. The refreshed earned-deflect endpoint assertion passed on the fresh binary, as did its ensuing deflection-count, no-death, and saved-ID assertions. The next uninterrupted earned dojo/hut route reached all asserted key, door, and transition events through the village hut.

The **first new failure** is `port/tests/integration.sh`'s `earned_dojo_hut` endpoint grep (line 359 at this checkpoint): it expects `village_hut.xml player=(-205,-21.5506) orbs=388 keys=2 health=1 .*balance=13`, while the actual result is the same position, 388 orbs, 2 keys, **health=4**, balance 13, spawned 18, `Deflection unlocked=1 count=1`, with no displayed death. The saved `earned-dojo-hut-save.txt` independently has 388 distinct orb IDs and the asserted `B:13`, dojo/hut keys, dojo/hut door flags, and deflection unlock. However, the script stopped at the health grep; its subsequent deflection, no-death, saved-ID Python checks, boat checks, and remainder were **not executed**. This is evidence of another stale endpoint-health expectation, not proof of exact combat parity. No further test assertion was changed.

Only the earned-deflect health change made by this rerun is staged for the scoped software commit. Earlier uncommitted integration assertion edits belong to the preexisting working tree and remain unstaged, although the run exercised them through the deflection route. No shared status/checkpoint, source, authored assets, physical GPU, or capture was touched.
