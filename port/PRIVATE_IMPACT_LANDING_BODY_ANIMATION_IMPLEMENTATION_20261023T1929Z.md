# Impact-landing body animation implementation — 2026-10-23

## Scope and source contract

Owner: exclusive `port/game/LevelScene.cpp/.h`, `port/game/PlayerMotion.cpp/.h`, `port/linux/main.cpp`, and `port/tests/impact_landing_body_cpu_20261023.cpp`. Starting HEAD `ce8e7ed` had unrelated dirty HUD, renderer, particle, test, status and report work; none was staged or modified here. HUD vertical-UV owner retains Hud.cpp/GPU. No GPU or visual capture was attempted.

Implemented the bounded handoff in `PRIVATE_FOREST_NEXT_NONHUD_VISUAL_SOURCE_AUDIT_20261023T1927Z.md`: load original `ImpactLandSoft` and `ImpactLandHard` XML frame paths/rates; pass motion recovery into the Player ID2 visual selector; hard then soft outrank melee, roll, crouch and ordinary states. The original `Character::UpdateAnimations` hard/soft early-return priority and `AnimationPart::Animate` single-step/past-last-frame completion are reflected in selection and finish detection. The last valid texture is retained on the completion update. Live play clears recovery on the visual finished edge; motion-only/simulate-only callers retain the prior timed fallback. Roll cancellation, side-collision cancellation, downward-dash hard impact, camera-shake event, arm/body painter order and existing scene renderer are unchanged.

## CPU verification

- Isolated host build/run of `impact_landing_body_cpu_20261023.cpp`: PASS. Forest-1 Player ID2 command spy checked soft texture order 005→000, hard six held 005 frames then 004→000, source 30/15 FPS step clock, final-frame completion, impact priority over melee/crouch, exactly-300 negative, short downward-dash hard, and successful-roll cancellation.
- Isolated existing `right_stick_camera_cpu_test.cpp`: PASS. This covers 300/700 thresholds, timer-only fallback, downward dash, hard side-collision cancellation and buffered roll.
- Isolated existing `crouch_animation_cpu_test.cpp`: PASS. Authored crouch/stand transition remains intact.
- `g++ -std=c++17 -fsyntax-only -Iport/compat port/linux/main.cpp`: PASS. `git diff --check` on owned source: PASS.
- No full SDL binary build or physical-GPU test: this owner was instructed CPU-only while concurrent HUD GPU lane remains reserved. No original-Windows matched-state frame, pixel comparison or visual acceptance is claimed.

## Owner release

Scoped software commit `2903a62b4502d5d96c47be3036665e5034287931` was pushed to `origin/feature/linux-renderer-foundation`; `git ls-remote` returned the same hash. This owner now releases only the files listed above. Hud.cpp, SDLRenderer.cpp, shared build outputs and physical GPU remain with their separate owners.
