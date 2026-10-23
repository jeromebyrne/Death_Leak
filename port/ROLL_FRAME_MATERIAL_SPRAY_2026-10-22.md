# Roll-frame material spray dispatch — 2026-10-22

## Change

`LevelScene::UpdatePlayerVisual` now dispatches the existing three-particle material footstep emitter after advancing a Roll body frame when the current frame differs from `lastRunningFrame_` and current solid-line contact exists. The emitter itself reads the authored `sfx_type="footstep"` frame flag and material texture, rejecting exact `water` and absent textures. Roll intentionally does **not** stamp `lastRunningFrame_`: source `Character::DoAnimationEffectIfApplicable` is called every rolling update against the last running-frame index, so a marked frame held across subframe updates may emit repeatedly. The direction comes from `PlayerStrafeVisual::movementDirection`, not body aim/facing or instantaneous velocity. Running/Strafe dispatch is unchanged.

## Evidence and limitations

- Source: `Game/src/GameObjects/Character.cpp:875-900,1119-1171`, authored Roll flags at zero-based frames 10 and 16 in `Game/XmlFiles/animation/player_anim_default.xml`, forest grassground line ID3 with `rock_particle.png`.
- CPU fixture: `port/tests/roll_frame_material_spray_test.cpp` exercises marked/unmarked frames, held marked-frame repeat, last-running-frame equality suppression, lost contact, origin, material texture, depth, particle count and movement-relative direction.
- `grounded` plus the current segment set by `PlayerMotion::Update` is the port's solid-line-contact representation; this does not prove exact behavior at buffered-roll/landing collision boundaries.
- Scripted input currently leaves `PlayerStrafeVisual` at its default direction rather than populating it from `movementDirection`; therefore scripted left-facing Roll spray direction is not fully represented through this CPP-only seam. Interactive input populates that DTO. This should be repaired separately at the script DTO handoff if required for a scripted visual comparison.
- No physical-GPU/matched-original Windows evidence is part of this change; no visual-acceptance claim.

## Verification

Focused host CPU build with `g++ -std=c++17 -O0 -Iport/compat`, linking the new fixture, `LevelScene.cpp`, `PlayerMotion.cpp`, and four tinyxml sources: PASS. The other owner's integration suite had released the lane at this point. A Docker image build was started afterward, but that owner immediately began another integration rerun; the image build was stopped to avoid overlap. The focused compile exercises the modified production translation unit, so an image rebuild is not required for this bounded CPU gate. Do not interpret the owner's concurrent integration run as coverage of this new fixture.
