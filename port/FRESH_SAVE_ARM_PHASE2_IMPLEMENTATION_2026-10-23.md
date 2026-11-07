# Fresh-save intro arm / Pull cadence implementation — 2026-10-23

## Source contract

Forest-1 Player ID2 uses `player_anim_default.xml`. Its body `IntroCutscene1` has ten frames at 15 fps; `IntroCutscene2` has twenty frames with authored `FrameRate=40` despite `MaxFrameRate=30` (`Game/XmlFiles/animation/player_anim_default.xml:1446-1480`). XML load stores the initial frame rate without invoking `SetFrameRate` (`Game/src/Animation/AnimationSequence.cpp:30-44`), so Pull animates at 40 fps. `AnimationPart::Animate` uses the preceding rendered modified delta and advances at most one frame per update (`Game/src/Animation/AnimationPart.cpp:195-239`). Completion follows the advance past index 19 while the retained visible body frame is `_019`. Both intro arm sequences are empty (`player_anim_default.xml:1532-1536`); after the first eligible intro animation update, the source marks the arm finished and `Character::Draw` omits it (`Game/src/GameObjects/Player.cpp:927-984`, `Game/src/GameObjects/Character.cpp:1888-1907`). This is a source/command-path contract, not a physical pixel measurement.

## Edits within exclusive ownership

- `port/game/LevelScene.cpp`: intro body uses `sequence.rate` directly (15/40) instead of clamping to XML min/max. Existing ordinary Running/strafe rate handling, prior-rendered-delta accumulation, one-step advance, and Pull terminal semantics remain unchanged.
- `port/game/LevelScene.cpp`: ID2 arm draw is gated to `IntroBodyPhase::None`. This suppresses the ordinary `Still` arm throughout Looped/Pull, including a phase-transition draw, while retaining ordinary arm-before-body order after intro.
- `port/tests/fresh_save_intro_animation_test.cpp`: actual scene texture-ID/path draw spy checks one expected body and zero arm commands across Looped and Pull, including first/last and past-end retained `_019`; checks ordinary Still arm returns after a successful-shot restart. A float reference clock checks 15/40 fps, nominal 60-Hz and variable preceding-rendered-delta streams, one-frame maximum per update, and first finished signal. ID2-only render filtering excludes unrelated effects. The scene-only test does not claim to validate the main-loop strict `>2.75 s` held-Interact gate or save handoff.

## Verification and integration boundary

Compiled the focused CPU test in `/tmp` with `LevelScene.cpp` and TinyXML only, without the shared build directory or GPU `:0`; it passed: `fresh-save intro animation CPU test passed`. `git diff --check` on the two owned source/test files passed. The existing shared tree contains unrelated dirty modifications, including another worker's `Combat.cpp/.h` and `main.cpp`; none were touched. No shared integration build, physical GPU capture, matched Windows comparison, commit, or push was performed. Physical visual acceptance remains unproven and requires the exclusive GPU owner plus a matched reference.

**Ownership release:** `port/game/LevelScene.cpp`, `port/tests/fresh_save_intro_animation_test.cpp`, and this sole implementation report are released now. No build or GPU ownership was acquired.
