# Encounter-roll integration gate recovery (2026-10-23)

## Cause

The prior `B:330` test route ran right past the spawned ninja. At frame 330 the player was at X=2612, the ninja remained near X=1980, and the summary showed `spawned=1 enemies=1 hits=14 kills=0`. `hits` includes player damage: the route took one enemy projectile, leaving only 13 knife hits. At 1.5 damage per knife, those 13 hits amount to 19.5 against the ninja's 20 health. The asserted kill therefore did not occur. This is a route/assertion mismatch, not evidence of an encounter-roll implementation failure.

## Scoped repair

In `port/tests/integration.sh`, changed only the encounter kill route from `B:330` to `B:220,F:120`: chase while firing, then stop and finish the stunned ninja. The existing `spawned=1 enemies=0 .*kills=1` assertion remains intact. Earlier unrelated dirty changes in the same script were preserved; no gameplay source or fixtures were changed.

## Verification

- Focused software-renderer run: `--play port/tests/fixtures/encounter_roll.xml --script B:220,F:120 --fixed-dt 0.0166667 --simulate-only --combat-state` — PASS, `spawned=1 enemies=0 hits=15 kills=1`.
- Full `sh port/tests/integration.sh`: **PASS (exit 0)**; final gate reports its overall PASS marker. Log: `port/build/encounter-roll-gate-recovery-full.log`.

No physical GPU/Xorg used. No commit made.
