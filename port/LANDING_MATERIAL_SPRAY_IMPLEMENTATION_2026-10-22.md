# Forest landing material spray — implementation (2026-10-22)

## Scope

Implemented the source-backed visual-only material landing effect in the Linux port. `PlayerMotion` records a valid airborne center-Y apex, measures apex minus resolved landing center Y once on the airborne-to-grounded transition, and passes the selected contact segment, position, facing and drop to `LevelScene`. Initial authored placement and resets cannot manufacture a landing spray. Collision selection, jump, dash and roll physics are unchanged.

`LevelScene` checks the selected nonvertical solid-line segment and its successfully loaded material particle texture. The scene owns a distinct landing spray vector, deterministic RNG stream, load reset, lifetime update/erase and player-depth draw. A box or smashable winner remains ineligible even if a textured line lies nearby; water is not broadly vetoed. No audio or vibration was added.

The two thresholds and visual parameter sets follow `Game/src/GameObjects/Character.cpp:409-490` and the directed-spray factory in `Game/src/Particles/ParticleEmitterManager.cpp:373-397,424-525`: 5 particles for `[300,700)`, origin at landing foot Y +35, spread .8, speed 1–1.5, lifetime .8–1.2, size 32, scale-to 5; 10 particles for `>=700`, origin at foot Y, spread .4, speed 3–5, lifetime .4–1, size 5–10, scale-to 15. Both use selected material texture, player depth, 1200x720 culling box, gravity .8, nonlooping, brightness .8–1, loop-time 1, spawn spread `(5,.2)`, fade .15/.9, and scales-by-lifetime. The Linux private RNG is deterministic but does not claim Windows stream identity.

## CPU verification

Focused Docker test `port/tests/landing_material_spray_test.cpp` passes for measured apex-to-center drops 299.9/300/699.9/700 -> no spray/5/5/10 particles, including right and left origins, all particle ranges, selected `grassground` -> `rock_particle.png` texture, depth, flags and lifespan sequence. It also checks no duplicate on sustained contact, no spray before contact, authored initial snap, absent material texture, equal-height solid-box winner, a buffered-roll landing, and a natural ledge departure/fall onto a lower textured line. Separate takeoff and footstep vectors remain empty in these falling cases. The exact `300` and `700` threshold cases use controlled starting centers and resolved ground centers, not a velocity surrogate.

Docker `make -C port -j2` passed (only existing TinyXML warnings). No physical GPU/Xorg, screenshot, capture or rendered-parity check was run. A separate post-jump GPU worker owns that gate; a later exclusive matched Windows/Linux GPU review is still required before visual-parity acceptance. This is a bounded software slice, not a full native-port acceptance.

## Recovery verification and delivery

Software commit `1e7d42d70b946ec5c192557876151532bcbd6c67` is on `origin/feature/linux-renderer-foundation` (confirmed by `git ls-remote`). On recovery, a forced clean compilation of the production binary passed in `deathleak-linux-build:bookworm` using `make -C port -B -j2` (exit 0; only pre-existing TinyXML warnings). The focused test was compiled in the same Docker image from `landing_material_spray_test.cpp` plus port game/TinyXML sources and exited 0 with `PASS: landing thresholds, variants, authored entry, roll buffer, absent texture`. No landing source defect was found; no source change was needed in recovery. This is CPU/build verification only, with no physical GPU/Xorg or visual-parity claim.
