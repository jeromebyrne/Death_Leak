# Forest→Waterfall transition HUD visual source gate — 2026-10-23 22:28Z

## Verdict

**No new, unowned source-backed first-frame HUD or draw-order mismatch established.** This is a read-only source verdict, not pixel parity. The fade alpha/timing correction, Waterfall ID46/entry rendering, and the port-only FPS readout remain in their existing lanes; no code change is nominated here.

## Route-specific composition evidence

- Original Windows renders world into the preprocess target, composites postprocess, then draws UI (`Game/src/program.cpp:293-318`). `UIManager::Draw` walks current screens in order (`Game/src/UI/UIManager.cpp:174-180`), and `SwitchToLevel` appends `gameloading` after the pre-existing HUD via `PushUI` (`Game/src/GameObjects/GameObjectManager.cpp:532-561`; `Game/src/UI/UIManager.cpp:237-267`). The active loading screen has only the oversized `blank_dark.png` splash; its icon is commented out (`Game/XmlFiles/UI/gameloading.xml:1-20`).
- Linux follows world → postprocess (`port/linux/main.cpp:1474-1477`), then film/bars/HUD (`:1497-1513`), then loading splash (`:1535-1547`). The ordinary HUD remains under the partially transparent splash, as in Windows. Both sides retain UI updates during the loading hold: Windows updates all current screens even with gameplay paused (`Game/src/Game.cpp:253-255`; `Game/src/UI/UIManager.cpp:81-102`), while Linux continues calling `hud.Draw` in the gameplay render branch (`port/linux/main.cpp:1423-1424,1497-1513`); `Hud::Draw` advances meter progress there (`port/game/Hud.cpp:49-52`). This establishes composition and update-path presence, not identical pixels or meter values.
- The Windows UI manager has a general after-screens interactable-icon pass (`Game/src/UI/UIManager.cpp:192-229`), whereas Linux ends the loading composition at the splash (`port/linux/main.cpp:1535-1547`). That caveat is already documented in `PRIVATE_LOADING_FADE_COMPOSITION_ORDER_SOURCE_AUDIT_20261023T2222Z.md`. The Forest exit itself is an automatic `LevelTrigger` (`Game/XmlFiles/levels/forest_exploration_1.xml:214-220`; `Game/src/GameObjects/LevelTrigger.cpp:21-39`), not an authored loading icon. Its `Update` does not enqueue an interactable; other nearby objects could still do so. Source alone does not establish an icon above the splash on this route.
- The Linux `FPS --`/FPS text at the bottom-left (`port/linux/main.cpp:1515-1519`) has no original-game HUD equivalent, but it is explicitly identified as a port-only diagnostic in `PRIVATE_FPS_HUD_SOURCE_QA_20261023T2038Z.md`; it is not newly owned or presented as a transition-specific defect.

## Bounded next gate

After the fade worker's correction, inspect one qualified, matched original/port **first partially revealed Waterfall frame** and one immediately preceding opaque frame, with ordered UI draws and registered HUD crops. If an original interactable icon is demonstrably queued on the loading frame, compare its draw after the splash against Linux and open only that icon-layer slice. Otherwise keep the current world→HUD→splash ordering; any residual needs a first divergent draw/state before another implementation task. Do not infer parity from this source audit or reopen ID46/entry sampling here.

## Ownership release

Only this uniquely named report was written. No code, shared status/index, build, Docker, capture, game, or GPU was touched. **Read-only source lane released.**
