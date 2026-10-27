# Clock-aware earned ending and live recovery — 2026-10-27

The source NPC cooldown initialization bug and its isolated live recovery are
covered in [NPC_COOLDOWN_VISIT_GATE_20261027.md](NPC_COOLDOWN_VISIT_GATE_20261027.md).
Runtime fix40527e3 retains the native120-second threshold and legitimate later
respawns. This followup adds ordinary route inputs and acceptance only.

## Deterministic encounter-clock acceptance — PASS

`make -C port earned-wall-clock-route-test` runs
`port/tests/routes/earned_wall_clock_ending.script` from the original Forest-1
spawn in one process per purchase/pre-boat/completion snapshot. No save load,
warp, restart/heal, injected state, direct hit or authored XML change is used.
The permitted script characters are ordinary movement/jump/fire/melee/interact/
purchase controls. The test-only clock advances the NPC encounter timestamp;
it does not reduce or bypass its cooldown.

| Evidence | Verified result |
| --- | --- |
| Native central suppression | Loaded at age116717ms; cannot wake when first reached later |
| Granary key | frame25681 |
| Earned purchase | frame31929;397 distinct authored IDs;375 paid;balance22 |
| Natural projectile deflection | one; earned unlock, not a pre-owned fixture |
| Dojo / hut keys | frames40768 /42982 |
| Repair tools | frame48476 |
| Boat / title /EndStory | frames56556 /57457 /58657 |
| Final progression |421 unique authored CurrencyOrb IDs, balance46, four keys, health1 |
| Combat |26 actual spawns,6 kills, one deflection; legitimate respawns remain enabled |
| Completion save | exactly DL6 /MENU /C:1 |

The harness independently checks all saved IDs against unchanged level XML,
uniqueness, key sets,375-orb conservation, survival, exact milestones and the
source-style progression wipe. The historical406-ID fast ending remains an
unchanged separate regression in the complete integration suite, which passed
on40527e3. Diagnostic dead/out-of-level candidates are not accepted clears.

## Ordinary traversal reconciliation

Bounded held-jump/fire detours traverse Forts, Forest-2 and central trigger
areas above their narrow authored bounds; two arena encounters use correctly
timed jump/fire. Controls resume before level exit areas, so survival while
falling out of the world does not qualify. The earlier successful forest
purchase is joined directly to the known ordinary onward suffix; no state is
loaded between them. All four keys and boat repair tools are earned normally.
This is deterministic reachability/combat evidence, not a human feel review.

## Actual live-clock replay — PASS

Collected process exit0. The same complete input file reached EndStory58657 in
**977.685 seconds**, alive with421 earned IDs, balance46, four keys, health1,
26 actual spawns,6 kills and one natural deflection. Purchase, key, boat, title
and ending milestones exactly match the deterministic run. Final completion
save is exactly `DL6\nMENU\nC:1\n`; no progression was loaded.

The normal60Hz wall-clock pacer ran **without** `--fixed-dt`, `--simulate-only`
or `--simulate-wall-clock`. Strict offscreen hardware OpenGL selected Intel
HD4000 (accelerated1/software0). No DISPLAY/X11 socket or greeter access was
used. SDL audio was dummy, so this is not speaker testing. Log and save:
`port/build/respawn-recovery/live-ending.{log,save}`. The earlier actual-clock
before/after pair reproduced baseline death12950 and fixed survival13000
without changing its ordinary route.

Measured offscreen present interval p50/p95/p99 was16.660/17.658/18.780ms,
maximum467.651ms; update+draw frame p99 was5.943ms with42 samples over33ms.
These data include loading stalls and do not establish physical scanout quality.
Read-only physical snapshot2026-10-27T08:20:56Z still shows slick-greeter,
1440x900 at59.90Hz, no >60Hz advertised mode and no detected gamepad.

Even successful offscreen live play is **not** physical-display/controller/
high-refresh release acceptance. An attended unlocked desktop, real gamepad
and >60Hz physical display remain external prerequisites. Human audiovisual
review, source parity and physical-display sustained GPU acceptance remain open.
