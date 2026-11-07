# Running-frame material footstep spray — source slice (2026-10-22)

## Contract and change

Windows `Character::UpdateAnimations` invokes `DoAnimationEffectIfApplicable` on the running branch; `player_anim_default.xml` Running frames 0 and 10 carry `sfx_type="footstep"`. A newly entered marked frame on a dry solid line with a material particle texture makes one directed three-particle spray at `(player X + facing*12, collision bottom + 17)`, player depth. The source call uses spread .1, bounds 1200×720, speed 2–4, lifetime .3–.7 s, size 40–50, gravity .5, brightness .8–1, scale-to 2.5 and fade-in/out .15/.7.

`LevelScene` now retains `sfx_type` alongside successfully loaded body frames, reads material particle paths from the shipped `materials.xml`, retains each solid-line segment's material, and creates a seeded scene-owned nonlooping spray only on an entered flagged Running frame with valid dry contact. `PlayerMotion` reports the actual winning grounded line segment (or no line when airborne/on a box); collision decisions and movement values are unchanged. Sprays update and drain through the existing particle implementation and draw after the player at kPlayer depth. The renderer already multiplies brightness into alpha. Forest grassground uses `Game/Media/rock_particle.png`; softwood uses its authored texture. This does not add strafe/roll or jump/landing effects, alter XML/art, or touch combat spray.

## Verification

- Source-only XML check: Running has 20 frames, marks exactly indices 0 and 10; grassground has one active `rock_particle.png`, softwood one `softwoodparticle2.png`.
- `git diff --check` on the bounded source files: pass.
- `g++ -std=c++17 -fsyntax-only` on `LevelScene.cpp`, `PlayerMotion.cpp`, and focused CPU test: pass (no binary or shared build lane).
- Added `port/tests/footstep_material_spray_test.cpp`: checks one three-particle emission on frame 0 and 10 entry, no same-frame repeat, origin/facing/depth/texture/parameter bounds, airborne/submerged/no-texture suppression, and physical line contact. **Runtime execution pending** the reserved build/GPU owner; this report makes no gameplay or Windows/Linux visual parity claim.

## Limits

The runtime spray uses an isolated deterministic random stream, not Windows' global `rand()` history. For materials with multiple active particle textures this bounded slice selects the first; grassground and softwood each have one. GPU same-camera foot-crop and matched Windows comparison remain pending. No commit or push: source syntax and metadata checks passed, but the focused CPU executable and visual gate have not run.
