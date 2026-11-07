# Waterfall-bottom ID69 floating platform — atomic physical-GPU gate

## Verdict

**Positive Linux pixel contribution and post-scene visibility; no original/port parity verdict.** At saved active frame 826 on physical Xorg `:0`, nouveau **NVE7**, the XML ID69 floating-platform draw (scene ordinal 7) changes **7,851** RGB pixels immediately across its draw, all in x=703..850, y=79..200. A same-binary visible/hide control changes **7,135** pixels after later scene draws and Sepia but before HUD, all in x=703..850, y=80..194. Left-field, right-field, and player controls remain byte-identical at both boundaries. Thus ID69 is genuinely visible in the Linux frame, though later composition attenuates it. No matched original D3D10 active-bottom capture exists; no source-proven original/Linux divergence or production fix follows.

## Reproduction and identity

Ignored, isolated output: `port/build/private-bottom-id69-atomic-gpu-20261024t0357z/`. `main.cpp` copies the existing ID23 diagnostic pattern but targets **only** path `Game/Media/objects/floating_platform_1.png` when scene draw count is 8 (ordinal 7); it captures pre/post BGRA32 BMPs and skips that one draw only when `DEATHLEAK_PRIVATE_BOTTOM_ID69_HIDE=1`. It does **not** modify shared `port/linux/SDLRenderer.cpp`, tests, Sea-2, or production source. Overlay SHA-256 `59f7b057c754fcfdebebc336f49948a06cffe3e40b5a05cd366fc2a278462afd`; executable SHA-256 `19c3fd3fe2b47ce6a0165a109b8647e97a0b11aae1a111c8d45f51c0225384cb`. Isolated `deathleak-linux-build:bookworm` build succeeded (`build.log`) against read-only current source.

Serial `nve7-visible/` and `nve7-hidden/` runs used the same executable, `deathleak-capture:post-foliage`, Xorg authorization, `SDL_VIDEODRIVER=x11`, `DRI_PRIME=pci-0000_01_00_0`, and `MESA_LOADER_DRIVER_OVERRIDE=nouveau`. Both logs prove accelerated OpenGL `vendor=nouveau renderer=NVE7`, GPU Sepia.fx initialization and exit 0. Each used `--play Game/XmlFiles/levels/forest_exploration_1.xml --script B:1800 --frames 827 --fixed-dt 0.016666667 --capture /out/nve7-{visible|hidden}/final.bmp --atomic-postfade-waterfall-packet /out/nve7-{visible|hidden}/packet.json`. Packets agree on active Waterfall-bottom, render/update 826, player `(833.383118,1370.51953)`, and all ordered scene-draw command dictionaries. Ordinal 7 equals the saved `private-bottom-fog-packet-20261024t0320z/packet.json`: destination `[700.437317,73.8939209,153.357437,129.179474]`, tint alpha 153/opacity 0.6, flipX false/flipY true, no rotation/repeat/UV shift. The immediately preceding full-frame BMPs are byte-identical (SHA-256 `1d18146d8e1d757defcda54dd194cd3a119e353338013f71d052dc849f150acc`); hidden post equals pre. This is the distinct upper-right platform, not the same-art ordinals 5, 6 or 8 and not the ID23 cave blur.

The copied diagnostic **pins** render/stored camera on frame 826 to `(438.31134,1445.68396)` because natural replay camera is wall-clock dependent. This is a disclosed Linux intervention, not a naturally matched original-game frame. Film phase is **not pinned**; unlike the earlier ID23 A/B, this NVE7 pair happened to retain zero residual in the final-frame control regions, but the pre-HUD stage is the reliable ID69 visibility measurement. An initial offscreen/Intel pair and X11/Intel probe are retained under separate directories for provenance; they are **not** the physical-NVE7 evidence used for this verdict.

## Lossless RGB comparison

Changed means any nonzero RGB channel; mean absolute RGB is on 0–255. Pillow comparison of saved 1280×720 BGRA32 BMPs:

| Boundary / region | Changed RGB pixels | Mean abs RGB |
|---|---:|---:|
| Immediate pre→post ID69, full frame | 7,851 / 921,600 | 0.473 |
| Immediate pre→post, platform box `[700,74,854,204)` | 7,851 / 20,020 | 21.784 |
| Visible→hidden post-Sepia/pre-HUD, full frame | 7,135 / 921,600 | 0.038 |
| Visible→hidden pre-HUD, platform box | 7,135 / 20,020 | 1.749 |
| Visible→hidden final post-overlay/pre-EndFrame, platform box | 7,015 / 20,020 | 1.401 |
| Each of left `[0,0,450,720)`, right `[900,0,1280,220)`, player `[842,338,964,482)` controls at all three boundaries | 0 | 0 |

The first original/port discriminator remains a provenance-qualified original D3D10 active Waterfall-bottom draw and immediate pre/post ID69 plus final-scene readback at matched state. No original pixel or command equivalence is claimed. `CHECKPOINT_STATUS.md` was already dirty under other ownership, so no append was safe. Both physical capture containers and isolated build container exited; **exclusive GPU/build lane released**. No commit/push.
