# Forest pause/modal visual-update contract audit — 2026-10-22

**Verdict: source-backed visual bug.** During the Linux paper or upgrade modal, `LevelScene::UpdateVisuals` continues advancing forest fog ScrollingSprite IDs 119/122. The equivalent Windows modal sets `mPaused`, so those game objects stop updating. No code, renderer, or test changes were made; this is a read-only source audit.

## Evidence

- `Game/XmlFiles/levels/forest_exploration_1.xml:1631-1646,1647-1662` defines fog IDs 119/122 as updateable ScrollingSprites (not `always_update`), with 45s/70s movement and 6s fades.
- Windows `Game/src/Game.cpp:308-333` sets `mPaused = true` for both text and upgrade modals; `:269-291` does so for the pause menu. `Game::Update` still calls `mGOMInstance->Update(mPaused, delta)` at `:170-180` (except damage-effect pauses), but `Game/src/GameObjects/GameObjectManager.cpp:205-257` gates weather and all game-object `Update` calls on `!paused`; `:260-265` similarly skips post-update. `Game/src/GameObjects/ScrollingSprite.cpp:23-59` advances lerp position/alpha only from `Update`. Thus fog's movement/fade is frozen through an actual Windows pause/modal.
- Linux `port/linux/main.cpp:573-600` routes paper/upgrade modals around gameplay input/update; `:929` and `:948` open them. Weather/rain are explicitly gated at `:1069-1075`. Yet `scene.UpdateVisuals(dt, ...)` is unconditional at `:1076-1079`, and `port/game/LevelScene.cpp:830-857` increments scrolling `scrollElapsed` and changes XY/alpha for in-zone updateable sprites. While a modal remains open and the player/camera are stationary, in-zone IDs 119/122 therefore move/fade on Linux but not Windows. The same unconditional call also advances particle and animated-sprite clocks (`LevelScene.cpp:795-819,884-899`), though those are outside this fog-focused finding.
- Linux Escape outside these modals exits (`port/linux/main.cpp:466`), rather than entering an equivalent in-game pause menu. The proven discrepancy is the **paper/upgrade modal** state, not a currently implemented Linux pause-menu state.

## Related visual clocks (do not freeze indiscriminately)

- Windows weather update freezes because it is inside `GameObjectManager::Update`'s `!paused` branch (`GameObjectManager.cpp:209-217`); Linux already gates weather/rain during modals (`main.cpp:1069-1075`).
- Windows parallax auto-scroll is different: `Game::Draw` calls `mGOMInstance->Draw` (`Game.cpp:354,407-408`), whose draw loop calls visible drawable objects (`GameObjectManager.cpp:302-330`). `ParallaxLayer::Draw` mutates auto-scroll texture coordinates using the last update delta (`Game/src/GameObjects/ParallaxLayer.cpp:37-62,114-155`), so a visible auto-scrolling layer can continue during a Windows pause. Linux advances `autoScrollYPhase` at `LevelScene.cpp:820-829` within `UpdateVisuals`. A blanket skip of `UpdateVisuals` during modals would incorrectly freeze this separate draw-time behavior. Shader time also progresses in Windows `Game::Draw` (`Game.cpp:371-375`), and Linux updates its shader clock before modal routing (`main.cpp:443-455`).

## Minimal fix and regression check recommendation

Add a modal/paused contract to `LevelScene::UpdateVisuals` (or split visual clocks): while `paperModal || upgradeModal`, skip the object-update-derived portions, including ScrollingSprite XY/alpha and weather-dependent/particle clocks, but retain the parallax auto-scroll phase that mirrors Windows draw-time mutation. Pass the **current** modal state at the call site (`main.cpp:1077`), taking care that opening/dismissing a modal mid-frame follows the Windows update order. Do not use `UpdateVisuals(0)` as the whole solution, since that would also freeze parallax.

A focused source-level/unit regression test should load `forest_exploration_1.xml`, place IDs 119/122 inside the update zone, snapshot scroll phase/XY/alpha, advance multiple paused-modal frames, and assert those values stay fixed; then unpause and assert advancement resumes without catch-up. If an auto-scrolling parallax layer is present, assert its phase still advances during those frames. No test/build/runtime execution was performed in this audit.
