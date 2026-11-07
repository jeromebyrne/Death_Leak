# Forest animation preceding-delta CPU render-command spy — 2026-10-23

**Verdict: confirmed bounded source-path discrepancy.** With the live port's `LevelScene` and forest XML/assets, the current-delta command stream emits the ID68 orb and player Still/Running texture one eligible update earlier than a preceding-render-delta stream at selected thresholds. Six of twenty controlled ticks emitted a different target texture. This establishes frame-selection semantics, **not** measured RGB severity or proof from an original Windows binary/frame.

## Isolation and provenance

- Before testing, inspected the dirty shared tree: `LevelScene.cpp/.h`, `main.cpp`, integration and status files already had competing edits. Made **no** edits to them, no shared build, no GPU/Xorg invocation and no commit. This report is the sole repository write. The disposable spy source/executable lived at `/tmp/forest_animation_delta_spy.cpp` and `/tmp/forest_animation_delta_spy`.
- Compiled the disposable CPU harness with `g++ -O1 -std=c++17 -UNDEBUG -Iport/compat`, linking the **live** `port/game/LevelScene.cpp`, `port/game/PlayerMotion.cpp` and four `Game/tinyxml` translation units, into `/tmp`, following the host-only fixture recipe in `port/tests/integration.sh:9-16`. `Renderer` was a path-to-`TextureId` spy: `LoadTexture` assigned IDs and `DrawSprite` recorded emitted commands; it never initialized SDL or a GPU. `DEATHLEAK_RENDER_ONLY_XML_IDS=2,68` kept only the focal sprites. Each tick captured `LevelScene::Draw` commands and classified exact `Game/Media/orb/...` and player idle/run body paths, ignoring unrelated arm/spray commands.
- Two independently loaded forest scenes received identical state/eligibility histories. Scene A received each listed modified delta as the animation `dt`; scene B received `0` on tick 0 and the *preceding completed render iteration's* listed delta afterward. The tick used to update the previous-delta slot was recorded after draw. This mimics the source's `Timing.cpp:8` zero initial slot and `program.cpp:149-177` `Update` → `Render` → `SetLastUpdateDelta` order. The reference was also computed by two independent float accumulators per sprite: add only when eligible, subtract one `1/rate` and advance **at most one** frame on crossing; reset the player accumulator and frame at sequence entry. Every emitted texture path from both live scenes was asserted against those independent expected frame indices. No helper-only assertion stands in for command capture.
- ID68 uses the actual `forest_exploration_1.xml:1108-1125` sprite and `orb_anim.xml:3-15` ten Still textures at 10 fps. Player ID2 uses the actual XML animation file (`forest_exploration_1.xml:33-51`), Still 20 fps and Running with `vx=360` world units/s → `clamp(abs(vx/60)*1.8*2.65,4,60)=28.62` fps. All supplied deltas are `0.008`, `0.012` or `0.016` seconds (≤1/60). Player is grounded and otherwise neutral; entry to Running occurs at tick 11, restart of Still at tick 15. For off-zone ticks 6–7, ID68 receives a camera at `(10000,10000)` during update and a near camera during draw, allowing its *frozen* emitted texture to be checked. Player is always eligible.

## Captured command frame indices

`C/P` means current-delta and preceding-delta **emitted texture index**, respectively; all 80 individual texture-path assertions passed. ID68 index `n` maps to `Game/Media/orb/{1+10n}.png`; player Still index `n` maps to `Game/Media/characters/player/idle_default/idle_default_{n:03}.png`, Running to `.../run_default/run_default_{n:03}.png`.

| Tick | dt | Zone | Player | ID68 C/P | Body C/P | Discriminator |
|---:|---:|:---:|:---|:---:|:---:|:---|
| 0 | .016 | in | Still | 0/0 | 0/0 | zero-initial prior slot |
| 1–2 | .016 | in | Still | 0/0 | 0/0 | below thresholds |
| 3 | .016 | in | Still | 0/0 | **1/0** | current crosses 0.05 s first |
| 4–5 | .016 | in | Still | 0/0 | 1/1 | preceding catches up |
| 6 | .016 | out | Still | 0/0 | **2/1** | orb clock frozen, player clock active |
| 7 | .016 | out | Still | 0/0 | 2/2 | orb remains frozen |
| 8 | .016 | in | Still | **1/0** | 2/2 | orb crosses 0.1 s on re-entry |
| 9 | .012 | in | Still | 1/1 | **3/2** | changed-delta phase |
| 10 | .008 | in | Still | 1/1 | 3/3 | prior catches up |
| 11–12 | .012 | in | Running | 1/1 | 0/0 | Running entry resets body frame/clock |
| 13 | .012 | in | Running | 1/1 | **1/0** | Running current crosses 1/28.62 s first |
| 14 | .012 | in | Running | 1/1 | 1/1 | prior catches up |
| 15 | .016 | in | Still | 1/1 | 0/0 | Still re-entry resets body frame/clock |
| 16 | .016 | in | Still | **2/1** | 0/0 | orb remains continuous across player restart |
| 17 | .016 | in | Still | 2/2 | 0/0 | prior orb catches up |
| 18–19 | .008/.012 | in | Still | 2/2 | 1/1 | changed-delta continuation |

Examples of actual path assertions: tick 3 player current `idle_default_001.png` versus prior `idle_default_000.png`; tick 8 orb current `Game/Media/orb/11.png` versus prior `.../1.png`; tick 13 player current `run_default_001.png` versus prior `run_default_000.png`. The spy exited `PASS; differing command ticks=6/20`.

## Source-backed next correction

The live port adds the **current** `dt` in `LevelScene.cpp:938-943,1137-1145`, and `main.cpp:1105-1109` passes that update's `dt` to both methods. Original `AnimationPart.cpp:169-196,201-237` adds `Timing::GetLastUpdateDelta()`; `Sprite.cpp:869-885` and `Character.cpp` use that part for these animations. A correction is warranted: introduce one owner for the previous **rendered, modified** delta, initialized to zero and committed at the source-equivalent post-render boundary, then pass it only to orb/player animation accumulation. Preserve current delta for physics, weather, shader clock, orb hover, unrelated waterfall sheet cadence and update-zone/paused behavior. Keep sequence reset, authored/velocity rate and one-step maximum. The previous slot must still advance on an iteration when ID68 is off-zone (as shown by its re-entry); it must not be an “orb's previous eligible dt” cache. Avoid binding it in this lane while `LevelScene` and `main` are concurrently edited.

For visual acceptance, obtain a registered original Windows frame/trace with the same delta history, clock, zone and sequence state, camera/viewport, effects and assets; a later exclusive GPU lane could then compare lossless ID68/player crops across these threshold ticks. Existing unmatched screenshots/video cannot rank this discrepancy against broader forest composition gaps.
