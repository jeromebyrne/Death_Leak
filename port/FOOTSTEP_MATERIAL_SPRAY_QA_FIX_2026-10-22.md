# Running footstep source-contract correction (2026-10-22)

## Corrections

- `LevelScene` now retains the last Running frame number (initialized to a sentinel on level load), compares it with the current frame on every Running tick, and stamps it on every Running tick. Still and other sequences do not clear the gate. This follows `Character::DoAnimationEffectIfApplicable` / `mLastRunFramePlayed` in `Game/src/GameObjects/Character.cpp`.
- Material particle exclusion is exact `water`, matching `Material::Material::IsWater()` in `Game/src/GameObjects/Material.cpp`; `shallow_water` keeps its authored `large_splash.png` particle texture. Removed `!inWater` from the particle emission gate. The port's current `inWater` visual input selects a Swim sequence, so this latter removal is a source-contract correction, not a claim that a submerged Running case was reached through that input.
- No audio behavior changed; no GPU/visual-parity conclusion.

## Verification

- Focused CPU executable compiled with `g++ -std=c++17 -O0 -Iport/compat` from `port/tests/footstep_material_spray_test.cpp`, `port/game/LevelScene.cpp`, `port/game/PlayerMotion.cpp`, and the four `Game/tinyxml` implementation files. Executed from repository root: `PASS: material Running footstep frame gate, contact, exact water predicate, seeded spray` (exit 0).
- Regression covers frame 0 -> Still -> Running frame 0 suppression, frame 10 -> Still -> Running frame 0 emission, XML-loaded synthetic `shallow_water` and exact `water` material on a solid-line contact, and existing spray geometry/contact assertions. The fixture is a temporary copy of the shipped forest level with its first material name changed; no shipped XML was modified.
- `git diff --check` passed on the three implementation files. Runtime test uses a capture renderer, not a GPU renderer.
