# Private current-tree visual gate: intro burst fade argument order (2026-10-23)

## Verdict

The highest-impact source-proven visible-composition risk in this bounded current-tree check is a **regression in the focal 70-particle intro sword-pull burst**: the current port assigns `fadeIn=.1`, `fadeOut=.8`, while the original call supplies **`fadeIn=5`, `fadeOut=1`**. This corrects, rather than repeats, the recent `FRESH_SAVE_BLOOD_BURST_POST_FLIP_VISUAL_AUDIT_1742_2026-10-23.md`, whose argument mapping is reversed. It is a source/port discrepancy, not a matched Windows pixel result. The X prompt and other forest composition audits were not re-opened.

## Exact source/port chain

- Both original 20/50 directed-spray calls end after `scaleTo` with `3.0f, 0.1f, 0.8f, 5.0f, 1.0f` and `1.75f, 0.1f, 0.8f, 5.0f, 1.0f` respectively (`Game/src/GameObjects/Player.cpp:1035-1058,1060-1083`).
- The declaration and implementation order is **`scaleTo, spawnSpreadX, spawnSpreadY, fadeInPercentTime, fadeOutPercentTime`** (`Game/src/Particles/ParticleEmitterManager.h:76-82`; `.cpp:390-397`). The implementation uses the middle `.1/.8` in X/Y spawn offset generation (`.cpp:442-475`), passes them to the spray constructor (`:515-526`), and sends the final `5/1` to the fade setters (`:540-541`). Thus the prior audit's claimed source `.1/.8` fades are definitively the spawn-spread arguments.
- Source `ParticleSpray::Update` tests fade-out only when `percentTimeLeft < 1 - fadeOut`, then otherwise sets early alpha to `(1-percentTimeLeft)/fadeIn` (`Game/src/Particles/ParticleSpray.cpp:233-258`). With source fadeOut=1, the fade-out branch cannot fire for a live particle with nonnegative time left; source fadeIn=5 gives alpha `elapsedFraction/5` throughout its short life. That low opacity appears intentional or at least is the actual checked-in contract; do not substitute a conventional normalized fade pair without a matched original capture.
- Current working-tree Linux creates the two recipes with matching 20/50 counts and burst textures (`port/game/LevelScene.cpp:1072-1086`) but sets `spray.fadeIn=.1f; spray.fadeOut=.8f` (`:1087-1095`). Its update uses the analogous two-branch formula (`port/game/ParticleSpray.h:40-60`), so the assignments alter visible alpha. The same particles are painted at the player seam (`port/game/LevelScene.cpp:1431-1442`). This was read amid concurrent dirty source; no edit/build/capture was made here.

## Concrete discriminator / next owner action

At fixed lifetime fraction 0.20 with brightness 1 and an opaque texel, source `5/1` gives alpha **0.04** (8-bit tint about 10), current port `.1/.8` gives **1.00** (255): about 25x more opacity before blending. At fraction 0.90, source gives **0.18**, port `.8` fade-out gives **0.50**. A one-particle renderer-spy at those two fractions should first assert the source values, then a frozen held-pull frame should isolate burst pixels with the same camera, seed and world zoom. If owner accepts source-code parity, restore `5.f/1.f` for these two intro recipes only, preserving the distinct `.1f/.8f` spawn offsets and the recently fixed horizontal U flip. A matched Windows same-phase frame remains necessary for image-fidelity acceptance; the source code alone does not prove perceived quality.

## Scope and release

Only this uniquely named private report was written. No source, tests, shared statuses, build, GPU, Xorg, or existing audit changed. **Read-only visual triage ownership and all execution lanes are explicitly released.**
