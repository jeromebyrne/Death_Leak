# Private sea_2 final-film integration patch packet

UTC: 2026-10-23T17:57Z. CPU/read-only source inspection plus this private report only. **Proposal, not an applied source patch or pixel-parity result.** The burst-surface GPU/build worker owns the dirty `port/linux/main.cpp` lane until explicit release.

## Evidence and discrepancy

- `Game/XmlFiles/UI/final_scene_hud.xml:3-16` defines a 1920×1080 quarter-alpha animated film from `film_frame.png`, `_2.png`, `_3.png`, `_4.png` at 11 FPS. `Game/src/GameObjects/Boat.cpp:91-93` switches to sea_2 and swaps `game_hud` for `final_scene_hud`; `Game/src/UI/UIManager.cpp:97-102` and `UIScreen.cpp:131-136` update active screens/widgets.
- `Game/src/UI/UIAnimatedSprite.cpp:4-7,61-77` initializes index/last-switch to zero, samples integer `timeGetTime()`, and increments **once** if `last + 1000 / float(11) < now`, wraps over textures, and stores the sampled `now`. For monotonic integer milliseconds this is first eligible at 91 ms, not 90; late updates do not catch up. `Game/src/Game.cpp:254` calls UI update before render.
- Current `port/linux/main.cpp:1352-1364` instead computes `int(finalSceneTime * 11.f) % 4`. That value derives from capped gameplay `dt` (`:645-655`) and also drives separate strict 15/35-second final-title/end gates (`:640-654`). The film selector can freeze or skip independently of original uptime cadence. Do not change `finalSceneTime` or its gates.
- Current `main.cpp:37-49,421-426,1365-1373` already has an 11-FPS monotonic one-step helper and state for ordinary `game_hud`; the original final screen is a **separate widget** and needs separate mutable state. Existing sea_2 four assets, 1280×720 draw, alpha byte 64, postprocess, 100-pixel bars and lower flip, and film→bars→title order (`:1349-1364`) remain intact.

## Minimal integration diff, to apply only after main.cpp owner release

Context is the observed dirty working tree, not HEAD. Add independent state beside `gameplayFilmIndex`; substitute the two-line scene-time selector in the sea_2 branch. Reuse the existing helper **without changing it**. This samples before the film draw, once per active final-HUD draw. Preserve state across a level transition in the same run because the original screen object is retained by `UIManager` (not recreated by `Boat::EnterBoat`).

```diff
--- a/port/linux/main.cpp
+++ b/port/linux/main.cpp
@@
     size_t gameplayFilmIndex = 0;
     std::uint64_t gameplayFilmLastSwitchMilliseconds = 0;
+    size_t finalFilmIndex = 0;
+    std::uint64_t finalFilmLastSwitchMilliseconds = 0;
     auto loadFilmFrames = [&] {
@@
-                const int filmIndex = static_cast<int>(finalSceneTime * 11.f) % 4;
-                if (filmFrames[filmIndex]) renderer->DrawSprite({filmFrames[filmIndex], {}, {0,0,1280,720}, {255,255,255,64}});
+                const auto finalFilmNowMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
+                    std::chrono::steady_clock::now().time_since_epoch()).count();
+                AdvanceGameplayFilmFrame(static_cast<std::uint64_t>(finalFilmNowMilliseconds),
+                                         finalFilmLastSwitchMilliseconds,
+                                         finalFilmIndex, filmFrames.size());
+                if (filmFrames[finalFilmIndex]) renderer->DrawSprite({filmFrames[finalFilmIndex], {}, {0,0,1280,720}, {255,255,255,64}});
```

The existing helper uses integer `1000 / 11 == 90` with strict `< now`, equivalent to the original float threshold for normal non-wrapping integral-ms samples. Its name is gameplay-specific but no rename is needed for this minimal patch. Do not reset the new state on `finalSceneTime = 0` or couple it to gameplay film state. If the UI lifecycle is redesigned to reconstruct the final widget, revisit state lifetime then. `steady_clock` epoch is only an uptime analogue, not a promise of identical Windows absolute phase or 32-bit `timeGetTime()` wrap behavior; normal-run cadence/one-step behavior is the acceptance target.

## Integration regression gates

1. **CPU clock-injected selector:** initial `(index,last)=(0,0)`: sample 90→`(0,0)`; 91→`(1,91)`; 181→`(1,91)`; 182→`(2,182)`; 273→`(3,273)`; 364→`(0,364)`. A first sample at 10000 advances only to `(1,10000)`; repeating 10000 does not advance. Sample once per active final-HUD update, before draw. Keep a distinct ordinary selector initialized `(0,0)` and assert it is unchanged throughout.
2. **Scene-clock independence:** hold `finalSceneTime` at zero while injected uptime crosses 91 ms: film advances, title/end do not. Advance `finalSceneTime` across 15 and 35 seconds while sampled uptime stays within an ineligible interval: film does not jump, title/end gates remain driven by scene time. Check no change to existing title/end strict comparisons.
3. **Recording renderer spy at branch:** assert `ApplyScenePostprocess` precedes the selected full-viewport `film_frame*.png` draw with tint `{255,255,255,64}`; then top `{0,0,1280,100}`, bottom `{0,620,1280,100}` with `flipY=true`, then optional title. Assert all four existing asset paths map to indexes 0–3 and the ordinary HUD draw path still uses its own alpha 51, distinct bars and state. A source-only fixture cannot claim this live branch is integrated.
4. **Owner-run integration:** after lane release, review the current diff to avoid overwriting concurrent changes; build/test the merged tree, run a controlled-index sea_2 active-HUD lossless GPU A/B on the physical renderer, and check title timing and bar coverage. A matched original-Windows frame, including widget phase, state, assets, viewport, camera/effects, and capture stage, is still required for original pixel parity; none is established here.

## Blockers and lane release

`port/linux/main.cpp` is currently dirty and owned by the burst-surface GPU/build worker; no source edit or integrated executable test is authorized in this lane. The preflight `port/PRIVATE_SEA2_FINAL_FILM_CPU_PREFLIGHT_2026-10-23.md` already reports a passing isolated CPU fixture, but that is **not** an integrated test of the above diff. No shared checkpoint/status, tracked code, build/GPU/Xorg, commit, or push was touched for this packet. **This private sea_2 final-film integration-proposal lane is explicitly released now.**
