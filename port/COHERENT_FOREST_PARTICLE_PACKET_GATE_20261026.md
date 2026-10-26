# Coherent Forest active packet: stable particle lifetime phase — 2026-10-26

Base: exact remote `343eb7c00e610327edd9a7ef228ff21b9d3ac110`; isolated detached `/tmp/deathleak-coherent-packet-20261026`. Primary checkout's unrelated dirty source was not built or staged. This is diagnostic-only packet interpolation, **not** a visible high-refresh mode.

## Exact held-command attribution

Authenticated physical Xorg `:0` via live LightDM Xauthority read-only Docker bind and matching local-family hostname; no cookie bytes printed. Nouveau/NVE7, SDL x11/OpenGL `accelerated=1 software=0 vsync=1`, 1280×720, fixed 1/60 Forest `B:125`. Initial frame 120: 93 painter-ordered commands, 45 sampled/48 held. The 48 holds were:

- 30 stable-generation emitted particles: 15 scene death debris (`texture=280`, ordinals 32–46), 15 runtime material spray (`texture=243`, ordinals 50–64). Position-only admission rejected changing lifetime square size and/or tint alpha. Identities and ordinal positions were stable. This is the dominant compatible moving family.
- Eight scene animation commands, ordinals 24–31, changed `texture=277→268`; the player body at 47 changed `236→237`. These are discrete sprite-frame changes, not legal texture interpolation.
- World sprite 68 changed tint alpha (`166→159`) and other non-positional state; shader-clock sprites 81–82 changed tint/rotation or related state. Film at 87 changed `393→394`. Those remain current until their authored continuous phase and film selection are separately proven.
- Five non-sprite painter/pass/HUD commands (Begin 0, Zoom 1, Postprocess 86, FillBatch 91, SpriteBatch 92) are not sprite-motion candidates and remain in the same ordinal/order/state.

The diagnostic `--mixed-hold-audit-frame N` prints only unsampled command identity, phase and draw-state fields at the selected frame. It has no default effect.

## Bounded implementation and safety

`ParticlePhaseAdmitted` samples a stable `TopologyKey` (scene spray or material-spray namespace, particle slot and generation) only when both sealed packets have the same complete emitted command topology, painter ordinal, camera factor and scene epoch. It rejects birth/death, slot reuse, cuts, nonfinite/discontinuous position, size jump >32 px, alpha jump >64, non-square size, texture/source/flip/RGB/opacity/gain/rotation/UV changes, and any foliage/pixel-wobble shader. Only destination x/y/width/height and tint alpha are sampled; original command order, blend/opacity, texture, shader state and fixed simulation remain untouched. `alpha=1` returns the current immutable packet exactly. Existing projectile guards remain.

Focused CPU mixed-command test covers both particle namespaces, midpoint size/alpha/motion, alpha=1 exact current, generation recycle, flip and large alpha discontinuity; presentation packet fixture also passed. Final isolated Docker production build passed, SHA-256 `72136bec901d0b4207ad4cbcc4f5d923f2aca596d169a6fd9540d50340f8a114`.

Final-binary physical Forest frame 120 sampled **75/93** commands (prior 45/93), including all 30 particles and all three projectiles, with 18 commands held. Full renderer scene/postprocess replay current and alpha=1 BMPs were byte-identical SHA-256 `e309144c232d44de3ea89cc7e30fe7959203b908a94bac675857ff95d866950c`, 0/921,600 RGB pixels changed. Alpha=.5 SHA-256 `d7d10160dfa3dd9c2497722f62bad6c99a5d461d8dd40032b9ede17cb7fbf728`, 535,316 changed pixels; this is whole-frame visual effect, not an FPS gain. At Forest→Waterfall-top frame 415, epoch 2, 0/40 sampled; current/alpha=1/alpha=.5 all byte-identical SHA-256 `c4dbcdb6f5ddb7d97eb2dc8909cd173c3dcd626c1787cded40e2495b61efb35c`. Evidence logs and BMPs: `/srv/openclaw/workspace/death_leak_analysis/coherent-packet-evidence/`.

## Decision and next distinct gate

**No visible runtime B and no serial active A/B/A.** The dominant compatible particle family is now fractionally sampled in diagnostic offscreen replay, but the active frame still holds the player/body animation, eight scene animations, film swap and three other changing world sprites while scenery/projectiles/particles move. The fractional composite is therefore not a proven coherent full active frame; timing/FPS measurements of a visible B would be invalid. The opt-in replay/audit flags remain off by default. Next gate: explicit actor/animation frame-boundary policy and source-backed continuous tint/rotation/shader phases for remaining ordinals 47, 68, 81–82, plus film-frame hold policy; prove representative moving packets and scene cuts on physical NVE7 before any visible switch. No 8K/high-Hz/60-FPS/sustained/thermal/VRAM/release acceptance.
