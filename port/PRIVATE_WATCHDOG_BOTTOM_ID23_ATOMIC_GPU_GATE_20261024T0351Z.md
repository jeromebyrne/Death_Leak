# Waterfall-bottom ID23 left cave blur — atomic physical-GPU gate

## Verdict

**Positive, localized Linux visual contribution; negative original-versus-port parity gate.** At active render ordinal 826 and frozen camera/player, the ID23 draw changes **197,144 scene-target RGB pixels** (21.40% of 1280×720), confined to screen `x=0..397, y=92..719`. A controlled same-binary hide/visible comparison preserves identical scene pixels immediately before ID23 and shows **187,356 changed pixels** in the left `[0,0,450,720)` region after later scene draws and Sepia/postprocess but before HUD/film; the right `[800,0,1280,720)` and player `[842,338,964,482)` controls remain **byte-identical** at that pre-HUD stage. The left blur is therefore genuinely visible in the final scene composition, not merely a submitted/offscreen sprite. There is still **no matched original-Windows ID23 packet or pixel reference**, so no source-proven original/port divergence and no production fix.

## Reproduction and identity

Evidence is confined to ignored `port/build/private-bottom-id23-atomic-gpu-20261024t0351z/`. Its `main.cpp` is a copied, opt-in diagnostic overlay, not shared source. It records BGRA32 scene-target readbacks immediately before/after renderer draw ordinal 23, gated on `Game/Media/levels/waterfall_bottom/cave_background_blur.png` and the 24th scene draw. The same overlay additionally permits an opt-in skip of that *single* inner draw for a final visibility control. Two serial runs use the **same executable** (SHA-256 `3cdebf3dd3367f80083ffcaf6cdfd7dfd246cf53a1a16ec6c09b35804841abd4`), visible vs `DEATHLEAK_PRIVATE_BOTTOM_ID23_HIDE=1`; overlay SHA-256 `c1251b34dda7efa2a44dd724d72b6ed79ce7df8b863ed34a355950113a0dd840`. The overlay pins the final frame draw/reported camera to `(438.31134,1445.68396)` because the unmodified lagged replay camera varies with wall-clock run duration. This intervention is disclosed and is **not** a natural exact original-game replay.

Docker build: `deathleak-linux-build:bookworm`, read-only current source plus isolated `/out/main.cpp`, successful `g++` build (`build-ab.log`). Serial captures: `deathleak-capture:post-foliage`, physical Xorg `:0`, `/dev/dri`, OpenGL accelerated renderer on nouveau **NVE7**, GPU Sepia.fx initialized, both exits 0 (`ab-visible/run.log`, `ab-hidden/run.log`). Each invoked:

`--play Game/XmlFiles/levels/forest_exploration_1.xml --script B:1800 --frames 827 --fixed-dt 0.016666667 --capture /out/ab-{visible|hidden}/final.bmp --atomic-postfade-waterfall-packet /out/ab-{visible|hidden}/packet.json`

Both packets identify active `Game/XmlFiles/levels/waterfall_bottom.xml`, render/update 826, player `(833.383118,1370.51953)`, camera above, and exactly equal command dictionaries for all checked ordinals 17, 19–26 and 34; each equals the corresponding frozen `port/build/private-bottom-fog-packet-20261024t0320z/packet.json` command. ID23 specifically is texture `cave_background_blur.png`, destination `[-726.772461,-7.8588562,1166.66675,875.333374]`, white RGBA/opacity 1, rotation `+10.906251°`, flipX/Y true, no repeat/UV shift. The visible/hidden **pre-ID23 full 1280×720 BMPs are byte-identical** (SHA-256 `98a9e75312155afb6e9b7b5ecec6cf5286fa76aaa39bfa2936b77f49c6521211`), making this an atomic pixel comparison. Hidden post-ID23 equals that same pre hash. Visible post-ID23 SHA-256 is `a062da01ab3d3c38d550dde5c6f84cf899904cc1834909a9a59a9d2544815785`.

## Pixel measurements and limits

Lossless RGB comparison, changed means any nonzero channel; mean absolute RGB change is on 0–255 scale. The left box is `[0,0,450,720)` (324,000 pixels); right control is `[800,0,1280,720)` (345,600); player box is `[842,338,964,482)` (17,568). Measured with Pillow `ImageChops.difference`/`ImageStat` on saved BMPs.

| Boundary / region | Changed RGB pixels | Mean abs RGB |
|---|---:|---:|
| Immediately pre→post ID23, full frame | 197,144 / 921,600 | 3.635 |
| Immediately pre→post ID23, left box | 197,144 / 324,000 | 10.338 |
| Immediately pre→post ID23, right control | 0 / 345,600 | 0 |
| Immediately pre→post ID23, player box | 0 / 17,568 | 0 |
| Visible→hidden post-scene-Sepia/pre-HUD, left box | 187,356 / 324,000 | 5.323 |
| Visible→hidden post-scene-Sepia/pre-HUD, right control | 0 / 345,600 | 0 |
| Visible→hidden post-scene-Sepia/pre-HUD, player box | 0 / 17,568 | 0 |

At the complete post-overlay/pre-EndFrame `final.bmp`, the left box differs at 240,122 pixels, **but so does the right control** (188,892), consistent with run-to-run film phase. Thus final BMP A/B is not an isolated ID23 estimator; the clean post-scene/pre-HUD readback is the final-scene visibility gate. `ab-visible/final.bmp.prehud.bmp` vs hidden pre-HUD shows the localized effect with zero right/player residual. The original Windows source/XML/asset command comparison remains as documented in `port/PRIVATE_WATCHDOG_BOTTOM_DISTINCT_VISUAL_GATE_20261024T0322Z.md`; this GPU experiment closes its previously unmeasured *Linux* pixel-contribution question only. The first original-versus-port discriminator remains a matched original D3D10 command plus pre/post-ID23 and final-scene readback at this state.

No shared production source, dirty renderer/test files, Sea-2, `PORT_WORK_STATUS.md`, git index, commit or push touched. `CHECKPOINT_STATUS.md` was already dirty under other ownership, so this worker did not append to it. Physical GPU/Xorg capture and Docker build containers exited; lane released.
