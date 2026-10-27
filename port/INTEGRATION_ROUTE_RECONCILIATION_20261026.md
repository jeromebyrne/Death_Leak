# Fixed-clock earned-route reconciliation

Continuation of `947691c`; changes confined to deterministic test inputs,
assertions and evidence. No gameplay constants, currency, damage, cooldown,
production collision geometry, source assets or authored levels changed.

## Cause and repair

The historical first crossing `R:710,E:40` falls into Waterfall-bottom on both
an exact independent upstream `4b29327` build and the controls binary.
`R:711,E:40` crosses; another 15 right-fire frames align subsequent jumps to
that 16-frame later Forest-2 entry. The second forest visit needs `R:566`
instead of `R:550`. The hut's `I:91,.:39` becomes `I:92,.:38`: the exact fixed
60-Hz float clock needs one further held frame to finish unlocking/entry.
The no-tools boat fixture extends I:4 to I:8 after R:43 to reach its first
post-warmup refusal at frame47. The isolated deflection discriminators move two frames later together so
phase1 reflects the naturally arriving star and phase2 tests that same
contact strip without reflecting; locked/wrong-facing/phase3 gates remain.
NPC jump/roll and completed-stun samples extend one tick, preserving actual
positive behavior rather than accepting a still-active stun. Printed velocity/
position rounding stays exact at observed values; keyed door entry is133.
The save-bridge village-smash route adds a normal held jump to catch its last
orbiting reward, retaining all8 credits, balance18,10 authored IDs, same final
position and roundtrip assertions; even600 idle frames did not catch that orb.
The synthetic health-pickup exit fixture formerly required an unreachable
50ms update (now capped). Its player starts at-55 instead of-85 so exactly
three pre-motion60Hz overlaps complete the hold, while the third post-motion
position lies outside overlap. Tests assert no collection after two visits,
collection on the third and the independent unheld post-motion geometry.
No original game-level XML or production threshold was changed. Failed candidates remain in ignored
`port/build/route-reconcile/` logs.

Updated assertions remain exact: transition frames, surviving endpoints,
unique-ID counts, 21/16-ID strict detour supersets, insufficient-funds refusal,
375-orb purchase/debit, persisted ownership, same-process ninja deflection,
four earned keys, boat gate, timed title/EndStory and DL6 progression wipe.
The authored pickup provenance audit additionally resolves every saved ID to
an actual CurrencyOrb in the unchanged level XML and checks conservation.

## Verified connected progression

- Purchase-only route: 375 distinct IDs, purchase frame31637, balance0.
- Same-run purchased deflection: 383 IDs, balance8, one deflection.
- Dojo/hut continuation: 388 IDs, balance13, both keys, alive.
- Full ending: granary25681; purchase33227 at384 IDs/balance9;
  dojo42066; hut44280; repair tools49774; boat57854; title58755;
  EndStory59955. Final406 IDs, balance31, four keys, health1, deflection1.
- Exact upstream4b29327 and controls binaries produce identical73 normalized
  progression/combat records and identical DL6 completion save on this route.
  Evidence: `port/build/route-reconcile/baseline-comparison.txt`,
  `upstream-full.log`, `full-v5.log` and their completion saves.

## Limits retained

The simulated-wall-clock cooldown gate still reproduces a village-central
respawn death (now12950, 12 spawned/two live). The fast earned ending does
**not** establish real-time survival. Do not bypass the native120-second
cooldown or call the port release-complete.

Read-only physical state: Xorg root is slick-greeter; LVDS1440x900 at59.90Hz,
no advertised >60Hz physical mode and disconnected external DP/VGA; input
inventory has built-in keyboard/trackpad and applesmc/js0, no verified gamepad.
Need attended unlocked desktop, real controller and >60Hz physical display
for those separate mandatory acceptance gates. No login/input was injected.
Prior strict physical NVE7 smoke remains valid only as a short GPU smoke.

## Integration/publication

Fresh Docker build and complete uninterrupted integration suite both exited0
on2026-10-27, including save bridge, streamed dummy audio and the authored-ID
audit. Independent bounded static review also passed. Full evidence and
publication/preservation checks are in CHECKPOINT_STATUS.md; the live remote
SHA receipt is port/build/route-reconcile/publication-receipt.json.
Parent workflow owns source-conversation delivery. This is not release acceptance.
