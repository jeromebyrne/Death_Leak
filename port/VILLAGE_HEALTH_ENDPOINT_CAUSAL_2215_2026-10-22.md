# Village-hut health endpoint gate — 22:15, 2026-10-22

Scope: read-only audit of existing reports, current dirty `port/tests/integration.sh`, the latest strafe closure trace, saved hut state, and commit scope. This report is the sole write; no build, suite, Docker, GPU/display, source, test, or shared status change.

## First failing gate and causal assessment

`port/build/strafe-footstep-integration-closure-2026-10-22.log:5374-5378` ends the continuous earned-purchase/deflection/dojo/hut run at `village_hut.xml player=(-205,-21.5506) orbs=388 keys=2 health=4 focus=100 balance=13 spawned=18 enemies=0 ...`, then fails the `health=1` grep. The exact stale expectation is `port/tests/integration.sh:368`; its preceding purchase, key, door, and transition greps ran through the hut door at frame 44830. The current `port/build/earned-dojo-hut-save.txt` independently has 388 distinct orb IDs, `B:13`, both keys, both door flags, and `U:kDeflection`. The log also displays `Deflection unlocked=1 count=1` and no death. The script stops before its *subsequent* deflection grep, no-death check, Python save assertions, boat checks, and remainder; those are not full-suite passes.

This is the same endpoint and values already reported as rerun9's first failure in `port/VILLAGE_GATE_RERUN9_2026-10-22.md`, before the strafe-footstep commit. Current HEAD `7aa97d9` changes only LevelScene strafe-footstep code and two CPU fixtures; the latest run preserves the pre-commit hut endpoint. The dirty script has progressively refreshed earlier route health expectations to 4, including the 383-orb earned-deflect endpoint, while leaving this line at 1. Existing `VILLAGE_ASSERTION_CAUSAL_TRACE`/`VILLAGE_INTEGRATION_EXPECTATION_AUDIT` document a plausible longer-knife combat mechanism for the 1→4 baseline but do not prove the exact avoided damage event. Thus the repeated hut result supports a stale endpoint assertion, **not an identified new gameplay regression or proof of exact original-game combat parity**.

## Minimal justified next action for integration owner

Change only `port/tests/integration.sh:368` from `keys=2 health=1 .*balance=13` to `keys=2 health=4 .*balance=13`. Preserve exact position, 388 orbs, two keys, balance 13, prior events, deflection count, survival and saved-ID/flag assertions. Then rerun the full integration suite in the owner's exclusive build/test lane and adjudicate the first newly exposed failure; do not blanket-refresh later health checks. No suite PASS is claimed by this audit.
