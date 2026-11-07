# Waterfall-bottom lower fog IDs 43–47: exact-frame occlusion gate — 2026-10-24 03:25Z

## Verdict

At active bottom render ordinal **826**, IDs **43–47 do not create a major player/aim-line visual occlusion** in the current Linux GPU frame. Same-render scene readbacks show ID45 produces no visible pixels, ID46 changes only lower-left pixels, and ID47 only upper-left pixels. A separate, bottom-level-scoped hide of all five changes **zero pixels** in the exact player and aim-line command rectangles after scene sepia/before HUD. IDs43/44 are authored but not submitted at this camera. There is **no rope texture draw in this packet**; this frame cannot prove anything about rope visibility at another time. This is Linux visual sensitivity/negative occlusion evidence, **not** original Windows parity or a source-proven defect. No production correction is justified.

## Reproduction and identity

Reused the frozen physical-Xorg/NVE7 diagnostic binary from `port/PRIVATE_WATCHDOG_WATERFALL_BOTTOM_FOG_SAME_RENDER_GPU_PACKET_20261024T0316Z.md`, SHA-256 `c44e7093987a69f9bf6bba945522b7664d0058c1adad21fe75071c30317569f1`. No fresh build while the independent performance build was active. Evidence remains in ignored `port/build/private-bottom-fog-packet-20261024t0320z/`. Baseline `packet.json`/`final.bmp.prehud.bmp` and new `hide-lower.json`/`hide-lower.bmp.prehud.bmp` report render/update ordinal 826, bottom level, loading phase 0, active gameplay/scene updates, and **identical camera `(438.31134,1445.68396)` and player `(833.383118,1370.51953)`**. Both are strict accelerated OpenGL on physical nouveau/NVE7 with initialized GPU Sepia scene-before-HUD. The hide uses `DEATHLEAK_RENDER_HIDE_XML_IDS=43,44,45,46,47` and `DEATHLEAK_RENDER_HIDE_XML_IDS_LEVEL=Game/XmlFiles/levels/waterfall_bottom.xml`; same route/script/fixed dt as baseline (`forest_exploration_1.xml`, `B:1800`, 827 frames, `0.016666667`). The isolated packet binary also needs `DEATHLEAK_PRIVATE_BOTTOM_FOG_PREFIX=/out/lowerfog` to permit a non-first-postfade bottom packet. A preliminary run omitted that opt-in and correctly exited 1 after its BMP, with no JSON; the corrected rerun exited 0. `hide-lower.json` SHA-256 `ceec6ff5e570ebac43fa415df9c529aa6c6866237a7977cce8ab5fdd5ee3a8cc`; its pre-HUD BMP SHA-256 `7bc07164312fef3d6e1004f9a5f8e57b19760b899f984c1339bd9daf941bbab5`.

## Source/command mapping and same-render pixels

`Game/XmlFiles/levels/waterfall_bottom.xml` authors IDs43–47 as 1368.948486×1011.360352 `fog_1.png` sprites at `kFarForeground`, alpha 1, `effectlighttexture`, no player-occlusion fade, rotation −0.199872 rad. `port/game/LevelScene.cpp:1382–1405` applies foreground parallax `(1.1,1.05)` and 2/3 screen scale. Matching computed destinations to live `packet.json` gives:

| XML ID | ordinal | screen destination `[x,y,w,h]` | same-render changed RGB pixels / bbox |
|---|---:|---|---|
| 43 | — | `[245.045,946.688,912.632,674.240]` | not submitted, below viewport |
| 44 | — | `[-83.489,1050.288,912.632,674.240]` | not submitted, below viewport |
| 45 | 31 | `[-138.342,769.378,912.632,674.240]` | **0** / none |
| 46 | 32 | `[-243.869,560.988,912.632,674.240]` | **25,121** / `[41,591,455,720)` |
| 47 | 33 | `[-75.422,-444.557,912.632,674.240]` | **119,409** / `[0,0,820,242)` |

Those pixels compare `fog.draw-{31,32,33}.pre.bmp` and `.post.bmp` around each submission in the **same render**, before sepia/HUD. All three have zero changed pixels in player-centered `[840,340,970,480)`, player-wide `[790,280,1020,550)`, and upper-right/aim region `[650,100,1050,390)`. XML ID45 can be submitted while its rotated, transparent texture leaves no quantized visible output; do not interpret the rectangular destination alone as coverage.

The ordered scene packet records the player frame at ordinal 17, destination `[842.917,338.326,120.929,143.568]`, and `aim_line.png` at ordinal 18, `[773,260,267,267]`; no `rope` texture path occurs in its draw list. Later far-foreground fog would overlay either if their sampled pixels reached it. They do not at this pose. The serial scoped hide changes **135,083/921,600** post-sepia/pre-HUD RGB pixels, bbox `[0,0,818,720)`, but **0/17,835** in floor/ceil player rect `[842,338,965,483)` and **0/72,361** in aim-line rect `[772,259,1041,528)`. Lower-fog effect is localized left: 111,368 changed pixels in `[0,0,820,243)`, 23,715 in `[0,590,456,720)`, and 41,924 in left control `[0,0,200,720)`. The hide is a separate process with matched reported state, not same-render A/B. Do not use final post-HUD A/B for fog attribution because wall-clock film phase can vary.

## Disposition

No matched original D3D10 Waterfall-bottom frame/draw/pixel packet is available; no first original/port divergent field was demonstrated. No XML/art/production renderer/Sea-2 files changed, no tests, commit or push. Only this evidence report and `CHECKPOINT_STATUS.md` were written; existing dirty files and `PORT_WORK_STATUS.md` preserved. Physical Xorg `:0`/NVE7 capture lane released: this task left no capture/game process running. A separate performance worker subsequently started its own capture; that worker and its build were not touched.
