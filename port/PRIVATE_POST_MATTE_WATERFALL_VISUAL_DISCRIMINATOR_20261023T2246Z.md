# Post-matte waterfall visual discriminator — 2026-10-23 22:46Z

## Verdict

**Concrete non-matte difference: the spendable-orb HUD reads 24 in Steam screenshot 16, versus 0 in the registered Linux Waterfall-top X315 frame. This is a gameplay/save-state mismatch, not a justified visual patch.** The number is drawn from currency balance in both source paths, not from a fixed Waterfall-top XML value. The screenshot and Linux capture have different player pose and unknown/known effect clocks respectively. Geometry registration of the raft/ledge does not normalize currency or collection history. No inference of renderer parity or HUD defect follows from this unmatched still.

This candidate is distinct from ID37/ID10 foreground occlusion, ID46 shaft mip sampling, loading fade, and the 32/33-row screenshot matte. It concerns the *value displayed by the HUD*, not orb hover/tracking art or the already-owned source/effect slices.

## Exact evidence

- Original reference `../store_media/screenshots/16.jpg`: 1920×1080 JPEG, SHA-256 `befba58c375f4b3c88d9b3b6224581fc37a5a621b54966520efc2497b0fda02c`. The registered-reference audit `port/WATERFALL_CAMERA_X_SWEEP_2026-10-23.md` observes HUD balance **24** and a different airborne player pose. This report uses that observation; it does not claim an independent matched-frame OCR or native D3D capture.
- Linux comparison `port/build/waterfall-camera-x-sweep-2026-10-23/x315.bmp`: 1280×720 lossless physical NVE7/Xorg diagnostic image, SHA-256 `846d7f2de43dbbd45e32f2303963c42f0f8a858fc8f9b6a826d621965e9c0b0b`; its companion `x315.log` records active `waterfall_top.xml`, fixed camera `(315,3000)`, first frame / shader time 0.0166667 s, player `(818.985,3233.32)`, **orb balance 0**, 81 loaded sprites, zero missing/unsupported. The archived diagnostic binary is HEAD `5c2d335` with an isolated fixed-camera override; it is not a current-HEAD exact-pixel baseline. The three raft/ledge landmarks fit after a common translation within 2.49 px RMS, but no gameplay state is matched.
- `Game/XmlFiles/levels/waterfall_top.xml:33-60` authors the direct-start player at `(818.985352,3234)`; `:99-108` separately authors the Forest-entry point at `(-2191.014648,3131)`. `:1270-1509` contains authored collectible CurrencyOrb objects. Neither the authored player start nor camera registration dictates a 24-orb spendable balance.
- Original `Game/src/UI/UIGameHudScreen.cpp:226-241` reads `SaveManager::GetNumCurrencyOrbsCollected()` each draw and formats that value at native UI rect `(105,135)`; `Game/src/GameObjects/CurrencyOrb.cpp:125` increments the save balance on collection. Thus screenshot 16's number is persistent gameplay state, not decorative texture content.
- Linux `port/linux/main.cpp:98,245-257` starts balance at 0 or loads the save's explicit `B:` balance (falling back to collected IDs for older saves); `:914-918` increments it for authored/spawned pickups. `port/game/LevelScene.cpp:630-645` collects authored currency objects within 100 world units of the player. `port/linux/main.cpp:1513` passes the balance to `Hud::Draw`; `port/game/Hud.cpp:47,84-91` draws the original orb icon and renders `std::to_string(orbBalance)` at projected `(70,90)` in 1280×720. The renderer then captures after HUD composition at `port/linux/main.cpp:1549-1552`. The Linux zero is consistent with its fresh direct-start diagnostic state; it is not a missing glyph or absent HUD evidence.

## Patch decision and bounded next action

**No justified patch.** Hard-coding 24, changing XML orb placement, changing HUD typography/position, or synthesizing collections to match a promotional JPEG would change gameplay without a first divergent draw/state field. The corrected 13-second current-HEAD Forest→Waterfall clip is a different route and render state; it cannot be substituted for the X315 pixel baseline or screenshot-16 gameplay history.

When original-Windows execution becomes available, capture one lossless post-HUD waterfall-top frame with an atomic packet containing executable/assets identity, save balance and collected-orb IDs, level entry, player/camera XY, render ordinal, UI projection/viewport, HUD text command, and native pixels. Reproduce **the same balance and state** in a current-tree Linux capture, then compare HUD icon/text geometry and glyph pixels separately from the scene. If the HUD draw fields agree but pixels diverge, isolate font/rasterization; if values differ, reconcile save/collection semantics first. The present 24-versus-0 comparison alone nominates no code owner or implementation.

## Scope

Read-only source/XML/evidence audit. Only this uniquely named report was written. No game source, XML, shared checkpoint/status, existing report, Git index, build, Docker, display/GPU/Xorg, capture, commit or push was touched. All pre-existing dirty files were preserved. Read-only lane released.
