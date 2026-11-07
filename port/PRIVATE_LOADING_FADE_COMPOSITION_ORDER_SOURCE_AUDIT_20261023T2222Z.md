# Forest→Waterfall loading fade — composition-order source audit

Scope: independent read-only source inspection of original Windows render/UI and the current Linux renderer. No edit to `port/linux/main.cpp`, build, capture, or GPU use. This report is the only file written. Timing, pause, and hold-boundary findings are owned by `PRIVATE_LOADING_FADE_INDEPENDENT_SOURCE_QA_20261023T2212Z.md` and `PRIVATE_LOADING_FADE_FIX_DESIGN_20261023T2214Z.md` and are not re-audited here.

## Verdict

**No concrete mismatch established for the four requested composition layers on this route.** In both implementations the world is sepia-postprocessed first, ordinary HUD is drawn unprocessed next, and the black `blank_dark.png` loading sprite is composited over both. The Windows loading overlay is the splash sprite itself; the loading code does not request a distinct black `fullscreen_fade` layer. The Linux full-viewport quad is source-backed by the authored oversized, uniformly black splash, but exact pixel parity is not asserted without a render comparison.

## Evidence and order

1. Windows `program.cpp:293-318` renders `Game::Draw()` into the preprocess target, calls `Game::PostDraw()` on the backbuffer, then calls `UIManager::Draw()`. `Game.cpp:428-445` selects `m_effectSepia` and draws the preprocessed scene. Therefore the UI (including HUD/loading sprite) is not sepia-processed.
2. `GameObjectManager.cpp:532-561` pushes `gameloading` via `UIManager::PushUI()` for the trigger transition. `UIManager.cpp:237-267` appends a newly pushed screen to `m_currentScreens`; `UIManager.cpp:174-180` draws that list in order. Thus the loading screen follows pre-existing game HUD screens. `gameloading.xml:1-9` contains one active sprite, `1_load_splash`, with `Media\\UI\\blank_dark.png`, `use_standard_effect="true"`, and a 2048×2048 authored rectangle; the second icon is commented out (`:11-20`). `UIGameLoading.cpp:19-37,40-57,60-93` finds that sprite and changes its alpha. No separate fade widget is created on this loading path.
3. Linux `port/linux/main.cpp:1474-1477` draws `scene` then calls `ApplyScenePostprocess()`. It draws film, frame bars, and HUD at `:1497-1519`, followed by the loading quad at `:1535-1547`. Its texture is loaded from `Game/Media/UI/blank_dark.png` at `:420-425`; the texture quad is viewport-sized rather than the authored oversize rectangle, which has no visible edge with a uniform-black texture covering the viewport. The fallback `FillRect` is only used when loading the image failed (`:1545-1546`), so it is not the normal art path.

## Narrow caveat, not a proven Forest→Waterfall mismatch

Windows `UIManager::Draw()` draws interactable icons **after** all current screens (`UIManager.cpp:192-228`), hence they can appear above a partially or fully black loading sprite if one was queued by `GameObject::UpdateInteractable()` (`GameObject.cpp:270-303`). Linux has no corresponding loading-phase icon pass after its splash (`main.cpp:1535-1547`). This is a real ordering difference in the general UI renderer, but the Forest→Waterfall trigger XML (`forest_exploration_1.xml:214-220`, trigger centered at x≈3411, y=3523) does not by itself prove that an interactable icon is queued at that location. Do not call this a demonstrated visual defect for the requested route without that evidence.

## Next bounded action

The implementer should finish the already-owned fade correction without a composition-order code change. If the Forest→Waterfall acceptance capture shows an icon on top of the Windows splash, make one focused comparison at the trigger: establish that `mInteractableIconsToDraw` is nonempty on a loading frame and then draw only that icon layer after the Linux splash. Otherwise leave composition order unchanged. No new fade timing work is implied.
