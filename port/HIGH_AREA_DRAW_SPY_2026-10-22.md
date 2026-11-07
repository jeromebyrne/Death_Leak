# High-area CPU draw-spy snapshot — 2026-10-22

## Result

**No missing admitted draw or single-field mismatch was proven** in the requested deterministic states. All 6 forest and 9 waterfall authored IDs generated a `Renderer::DrawSprite` call, in descending-depth painter order (XML order at equal depth). The source-only high-area omission lane stays closed. This is a CPU draw-state check, not a raster/pixel or Windows parity claim. Do not patch source from this result.

## Reproduction / boundary

- Inspected Git HEAD `daef1a0`, current dirty worktree, and latest `PORT_WORK_STATUS.md` ownership before running. The separate landing-spray A/B worker `f1beb4a7-e01d-42c2-894e-346ad950b1c0` retains exclusive GPU/Xorg ownership. No Docker, GPU, Xorg, ffmpeg, game binary, or source/test/shared-status edit was used.
- Compiled an ephemeral `/tmp/deathleak_draw_spy.cpp` with host `g++ -std=c++17 -O0 -Iport/compat`, `port/game/LevelScene.cpp`, and the four `Game/tinyxml/*.cpp` translation units. Its fake `Renderer::LoadTexture` returns unique IDs and `DrawSprite` records the complete `render::Sprite`; no texture decoding/rasterization. The harness accesses `LevelScene::sprites_` via a **harness-only** `#define private public` inclusion, then joins each requested XML ID to the emitted draw by texture, destination X/Y and size. It checks actual emitted calls, not simply loaded object fields. Source is unchanged.
- Forest: load `Game/XmlFiles/levels/forest_exploration_1.xml`; 98 updates at float `dt=.0166667`, camera follows *pre-motion* player X by `(playerX-cameraX)/15`, player advances `min(update,8)` world units each update, camera Y fixed at 3400; call `UpdateVisuals` once per update and advance/draw `ShaderClock` in main-loop order. Final camera `(593.214233,3400)`, player `(712.985352,3385)`, total/pixel clocks `(1.633335,1.616668)`.
- Waterfall: load `waterfall_top.xml`; camera `(0,3000)`, player `(818.985352,3234)`; snapshot before update, then call `UpdateVisuals(.0166667,...)` and `ShaderClock::Update/Draw`. Frame-0 clocks `(0,0)`; frame-1 clocks `(0.016667,0)` (pixel clock intentionally lags the first update). This is a controlled fixed-camera initial/frame-1 state; no gameplay movement is simulated.
- Destination values are logical 1280×720 pixels, `(x,y,w,h)`. `A` is emitted float opacity; `tA` is emitted 8-bit tint alpha. `UV` is emitted normalized `(U,V)` shift. `FX` lists foliage/pixel-wobble flags. “Draw index” is the zero-based call index among all scene `DrawSprite`s, including non-target IDs. Shader times on every row equal the state clocks above; for pixel-wobble-enabled fog/sheets the second clock is active. Rotated cull is included because these are actual downstream draw calls.

## Forest-1 R:98 draw state

| ID | depth/order → draw index | destination x,y,w,h | A / tA | UV U,V | flip X,Y | FX | scroll elapsed/time; fade-in/out |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 38 | 5500/38 → 4 | -6118.076,-674.741,13333.334,1828.923 | .150000 / 38 | 0,0 | 1,1 | 0,0 | — |
| 39 | 4750/39 → 11 | -6292.819,-564.461,13333.334,1828.923 | .200000 / 51 | 0,0 | 1,1 | 0,0 | — |
| 119 | 1750/89 → 46 | 132.712,227.333,1325.333,860 | .136111 / 35 | 0,0 | 0,0 | 0,1 | 1.633335/45; 6/6 |
| 122 | 1750/90 → 47 | -513.153,289.333,1325.333,860 | .163334 / 42 | 0,0 | 0,1 | 0,1 | 1.633335/70; 6/6 |
| 37 | 1000/37 → 49 | -42.667,589.333,1365.333,341.333 | .950000 / 242 | .409318,0 | 0,1 | 0,0 | — |
| 40 | 1000/40 → 50 | -42.667,-332.333,1365.333,451.333 | .950000 / 242 | .409318,0 | 0,1 | 0,0 | — |

Forest IDs 119/122 have source scrolling original alpha `.5/.6`; at elapsed `1.633335 < 6` their emitted opacity is original-alpha × elapsed/6. ID37/40 follow-camera X and share U shift `cameraX × .000690` (rounded). Their near-black/upper-edge visual contribution is not established by this CPU spy.

## Waterfall-top initial and frame 1

| ID | depth/order → draw index | destination at frame 0 | A0 → A1 / tA0 → tA1 | V0 → V1 | flip X,Y | FX | scroll elapsed/time; fade-in/out at frame 1 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 6 | 4750/6 → 7 | 569.554,-810.351,560.872,2202.092 | .5 → .5 / 128 → 128 | 0 → .004667 | 1,1 | 0,0 | — |
| 13 | 4750/13 → 8 | 574.436,-757.081,540.442,1080.884 | .3 → .3 / 77 → 77 | 0 → .007333 | 1,0 | 0,1 | — |
| 14 | 4750/14 → 9 | 572.436,323.585,540.442,1080.884 | .3 → .3 / 77 → 77 | 0 → .007333 | 1,0 | 0,1 | — |
| 58 | 4750/58 → 16 | -5844.677,-3110.667,13333.334,6666.667 | .2 → .2 / 51 → 51 | 0 → 0 | 1,1 | 0,0 | — |
| 117 | 1750/97 → 86 | -1139.343,14.667,1325.333,860 | .6 → .005 / 153 → 1 | 0 → 0 | 0,0 | 0,1 | .016667/70; 2/6 |
| 118 | 1750/98 → 87 | -758.010,122,1325.333,860 | .6 → .001667 / 153 → 0 | 0 → 0 | 1,1 | 0,1 | .016667/110; 6/6 |
| 119 | 1750/99 → 88 | 984.923,10.667,1325.333,860 | .6 → .001667 / 153 → 0 | 0 → 0 | 1,0 | 0,1 | .016667/70; 6/6 |
| 121 | 1750/100 → 89 | -808.677,1.333,1325.333,860 | .5 → .001389 / 128 → 0 | 0 → 0 | 1,1 | 0,1 | .016667/59; 6/6 |
| 31 | 1500/31 → 96 | 398.734,-1059.785,846.512,3323.570 | .2 → .2 / 51 → 51 | 0 → .004667 | 1,1 | 0,0 | — |

All waterfall U shifts are zero at camera X=0. Destinations of IDs 6/13/14/31/58 are unchanged at frame 1; moving fog X positions advance slightly: ID117 `-1139.145`, 118 `-757.969`, 119 `983.522`, 121 `-808.789` (same Y/size as frame 0). ID58 retains `repeatX=1`; other targets have `repeatX=0`. Fog frame-0 opacity is its loaded XML alpha, while the first update enters its authored fade-in. `tA=0` for three fog rows at frame 1 is rounded low alpha, **not** a missing draw; their nonzero float opacity remains in the emitted sprite. Curtain V phases match the port formula `speed × (720/1080) × .001 × (dt × 60)`.

## Next discriminator

After the exclusive GPU lane is released, same-camera ID-only crops may show each ID's Linux pixel contribution. A Windows-vs-Linux parity claim still needs an original frame with matched camera/player/weather/animation and shader/scroll clocks. No source correction or commit is warranted from these CPU states.
