# CurrencyOrb animation update-zone fix — 2026-10-22

Source basis: `port/FOREST_ANIMATION_PARITY_AUDIT_2026-10-22.md`. Windows `GameObjectManager::Update` calls `Sprite::Update` only for updateable objects within the camera-centered 4000×4000 world zone (or `always_update` objects). `AnimationPart::AnimateLooped` advances its per-frame clock only when called. Authored Forest 1 CurrencyOrbs are updateable and not always-updated.

Change: `LevelScene::UpdateVisuals` now applies that same existing world-space zone predicate before advancing a loaded sprite animation. Both the selected frame and fractional frame time hold outside the zone; a returning orb continues from its saved frame clock. No player descent timing change or other animation behavior was made.

Verification: Native non-GPU C++ tests compiled against `LevelScene.cpp` and TinyXML and passed:

- `port/tests/orb_animation_zone_test.cpp`: authored Forest 1 orb ID 68, sub-frame accumulation, freeze outside zone, resume on re-entry, and no off-zone catch-up.
- `port/tests/occlusion_fade_test.cpp`: neighboring authored tree fade/update-zone regression.

The focused test initially used an exact floating-point edge and then an exact 0.1-second threshold; those assertions were sensitive to rounding. It was corrected to probe slightly inside/outside and to cross the frame threshold unambiguously. Final runs passed. No display/GPU capture, Docker build, or visual acceptance was performed in this lane.
