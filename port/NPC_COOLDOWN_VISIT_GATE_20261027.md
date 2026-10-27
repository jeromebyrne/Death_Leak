# NPC cooldown visit-lifetime correction — 2026-10-27

## Genuine bug and source basis

The old central-return death was **not a legitimate respawn for that visit**.
The earlier diagnosis looked only at time when the trigger was reached.
`NPCTrigger::Initialise` instead removes a cooling trigger at level load;
`Update` immediately rejects removed objects. `GameObjectManager` initializes
all loaded objects after content loading, independent of player overlap.
A removed trigger cannot wake when its timer expires later in that visit.

Port collision-time checking admitted central#151 at age129850ms even though
its level had been installed at age116717ms. This caused two unwanted ninjas
and the frame12950 death. Independent source audit is preserved at
`port/build/respawn-recovery/source-audit.md`.

## Narrow production fix

`Combat::InitializeLevelEncounters` snapshots cooldown eligibility after
initial history/clock setup, menu New Game, every successful destination
installation and death restart. `ClearLevel` resets only visit-local state;
spawn history survives. Cooling IDs are consumed for that visit. Collision
only fires a surviving trigger and records its actual spawn timestamp.
CPU callers have a first-update fallback before synthetic clock advancement.

The120000ms threshold, missing-history behavior, future-history suppression,
one-shot-per-visit rule, spawn counts, damage, geometry, controls, saves and
original earned route are unchanged. A later visit initialized at or above
120000ms still respawns. Failed scene loads do not create a new visit.

## Verified acceptance

- Build PASS; complete uninterrupted `sh -x port/tests/integration.sh` PASS.
- New CPU test: suppressed visit stays removed across expiry; later reload
  respawns; exact120000ms eligible; future timestamps suppressed; once per
  visit; missing history allowed. Actual spawn time updates history.
- Unchanged ordinary route, real wall-clock60Hz pacer, no fixed-dt/test clock:
  baseline dies12950 after216.715s; corrected13000-frame run survives at
  central `(3167.18,-1223.81)`, health4,228 earned IDs/balance,10 spawned,
  no enemies, after216.723s. Actual load age116743ms was suppressed.
- Both live runs select strict **offscreen Intel HD4000 hardware OpenGL**.
  They open no X11 window and do not bypass or inject input into the greeter.
  This is genuine live-clock/ordinary scripted-input evidence, **not** physical
  display, attended controller or physical high-refresh acceptance.
- Existing earned375 purchase, same-run deflection,406-ID/four-key ending,
  metadata wipe, save bridge, audio and input regressions remain green.

Evidence: ignored `port/build/respawn-recovery/{build,cpu,integration,
baseline-clock,baseline-live,fixed-prefix,fixed-live,fixed-full-clock}.log`.
Binary SHA256:2025e377f499117b3ef60a14f1b091e3ddaf31637bee73c9ab9cbace026b3722.

## Distinct expected gameplay and open acceptance

The unchanged long route later reenters Forts at age167617ms: its5-ninja
respawn is legitimate and remains enabled. Normal jump/fire route exploration
can bypass that trigger, retain health4 and reach the barn; an added normal
Forest-2 return jump sequence advances to the granary key and second arena
visit around frame29563. That later arena fight is also legitimately eligible.
These route explorations are separate from the source bug fix; failed/dead/
out-of-level candidates are retained and are not acceptance evidence.
The original fast ending must not be called a complete real-time clear.

Physical attended gameplay, real controller and >60Hz display remain external
prerequisites. Human visual/audio parity and sustained physical-display GPU
acceptance remain unverified. The native port is not release-complete.
