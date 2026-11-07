# Forest→Waterfall scene-transition visual source audit — 2026-10-23 22:07Z

## Verdict

**One distinct, unowned source-backed visual discrepancy:** the original Windows game draws a full-viewport dark loading-splash fade over the Forest→Waterfall level switch; the Linux port replaces the scene immediately, with no corresponding transition overlay or fade. This is a transition-only composition gap, not a claim about steady-state Waterfall pixels or matched-Windows parity.

## Source chain and active-gameplay relevance

- `Game/XmlFiles/levels/forest_exploration_1.xml:213-221` gives right-edge LevelTrigger ID13 destination `waterfall_top.xml`. `Game/src/GameObjects/LevelTrigger.cpp:30-43` requests deferred `SwitchToLevel` on player overlap. `Game/src/GameObjects/GameObjectManager.cpp:282-286,532-561` resolves that request after object post-update, pushes `gameloading`, pauses gameplay, and starts loading.
- `Game/XmlFiles/UI/UI.xml:6` registers that screen; `Game/XmlFiles/UI/gameloading.xml:1-9` authors a 2048×2048 `Media/UI/blank_dark.png` splash as the sole active sprite. The icon below is XML-commented out. `Game/src/UI/UIGameLoading.cpp:40-93,105-135` starts at alpha 0, fades the splash in over 0.20 s, loads the new level once fully opaque, then fades it out over 0.20 s (with a minimum loading interval of 0.05 s). These are source-clock thresholds, not an assertion of exact presented-frame count.
- `Game/src/Game.cpp:253-255` updates the UI each frame; `Game/src/program.cpp:293-317` draws UI after the world and its postprocess. Thus the fade covers the outgoing Forest and incoming Waterfall as a screen-space layer, outside world postprocessing.
- In the port, `port/linux/main.cpp:1202-1231` loads/replaces the Forest scene at the same trigger, resets camera/player, and proceeds to the ordinary render block at `:1292-1427`. The only `blank_dark.png` load in `main.cpp` is the modal background at `:420`; no transition state/overlay is drawn around the switch. `:1313-1450` renders the active scene and HUD. Accordingly the port presents a direct cut at this active Forest→Waterfall gameplay boundary, where the original has the authored fade.

## Ownership / discriminator

Recent `PORT_WORK_STATUS.md` through 22:07Z and scoped visual reports cover Sepia, HUD/film, ID10/37 foreground, Waterfall ID13/14 curtains, ID46, raft mip, and per-sprite effects. A report-text search found no prior `gameloading`/`UIGameLoading` transition owner; `NEXT_EFFECT_CURRENT_HEAD_SOURCE_AUDIT_2026-10-22.md` merely mentions loading generically. This finding does not reopen those other lanes. The concrete implementation target is a screen-space `blank_dark.png` transition after scene postprocess, with the original 0.20 s in/out and level replacement under full opacity; first verify the `blank_dark` asset dimensions/alpha and original UI coordinate mapping before committing geometry. A later controlled capture should include the last Forest frame, opaque boundary, and first Waterfall frame; steady-state Waterfall capture alone cannot discriminate this gap.

## Scope / release

Read-only source/report inspection only. Sole write is this unique report; no source, tests, index, shared status, build, Docker, GPU/display, capture, commit, push, or unrelated dirty-file changes. **Read-only visual lane explicitly released.**
