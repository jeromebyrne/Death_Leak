# Fresh-save intro body animation seam — 2026-10-23

## Scope and result

Implemented only the LevelScene animation seam from `FRESH_SAVE_INTRO_IMPLEMENTATION_PREFLIGHT_2026-10-23.md`. The player body loader now reads `IntroCutscene1` (10 authored frames) and `IntroCutscene2` (20 authored frames). `SetIntroBodyPhase` selects the loop or pull animation explicitly; `IntroBodyFinished` becomes true only after phase 2 advances past its final visible frame. Phase 1 loops at its authored 15 fps. Phase 2's authored 40 fps is clamped to its XML `MaxFrameRate=30` and never loops. The clock uses the preceding rendered modified delta and advances at most one frame per update, matching the existing scene convention and original AnimationPart behavior. A level load resets the phase/signal; ordinary body selection remains the default.

The caller must determine genuine fresh-save forest-1 eligibility, held-Interact time, control gating, zoom, and phase transitions. No main/renderer integration was made. This seam alone does not activate the intro in gameplay or establish visual parity.

## CPU proof

`port/tests/fresh_save_intro_animation_test.cpp` loads the shipped forest-1 XML with a null renderer and checks 10/20 loaded frame counts, phase-1 15 fps loop at 60 Hz, phase-2 30 fps capped cadence, one-shot terminal after the last visible frame, terminal latching, reset, and preceding-rendered-delta priority. Compiled with g++ C++17 against LevelScene and shipped TinyXML; executable passed. `git diff --check` passed. No physical GPU, `:0`, or capture was used. Repository build target was not invoked because another owner holds build/display scope.

## Ownership release

Owned files: `port/game/LevelScene.cpp`, `port/game/LevelScene.h`, this report, and `port/tests/fresh_save_intro_animation_test.cpp`. These file and CPU-test scopes are released after the scoped commit. Build/display/capture scope was never acquired. Pre-existing dirty main/renderer/integration/checkpoint files and other untracked reports were untouched.
