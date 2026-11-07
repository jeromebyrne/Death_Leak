# Post-knife ID37 physical-GPU discriminator — 2026-10-23 21:00Z

## Verdict

**No new source-proven ID37 renderer parity fix.** A fresh current-working-tree strict hardware capture of active Forest-1 play makes the ragged lower dark foreground disappear when *only the render submission* of XML ID37 is suppressed. The phase-registered pair changes 51,801 RGB pixels, all in `[0,596,1269,687)`; the entire `[0,0,1280,590)` upper region is byte-identical. This is strong Linux causal attribution to authored shrubbery, not evidence that the original Windows image differs. No XML, PNG, source, test, shared status, index, commit, or push was changed.

## Frozen current-tree build and capture

Preflight HEAD `410b14622956bdfd42efea307c8de9ea03604d4c`; `port/linux/SDLRenderer.cpp` was already dirty and was included without editing, while `PlayerMotion.cpp` and `LevelScene.cpp` were clean. No other Death Leak Docker/game/Xorg-recorder process owned the lane. Built the live tree via `deathleak-linux-build:bookworm` with source bind mounted **read-only**, `make -e -C port TARGET=/out/deathleak-linux -j2`, into unique ignored packet `port/build/private-post-knife-id37-20261023t2100z/`. Build exited zero; executable SHA-256 `71b5bbc516803d176ea47a94e06d67ab0be6682194ec1ff46d85a15312a0b583`.

Three sequential captures used the **same executable** and source/assets, physical Xorg `:0` socket, LightDM Xauthority, `/dev/dri`, `deathleak-capture:post-foliage`, SDL X11/OpenGL/dummy audio, and `--play Game/XmlFiles/levels/forest_exploration_1.xml --script R:180 --frames 180 --fixed-dt 0.0166667 --capture /out/<mode>.bmp`. Runs were baseline, `DEATHLEAK_RENDER_HIDE_XML_IDS=37`, baseline repeat. The switch is read at `port/game/LevelScene.cpp:79` and suppresses the ID37 draw only; simulation is not altered. Every log reports `accelerated=1 software=0 mode=strict-accelerated`, OpenGL `nouveau/NVE7`, GPU Sepia scene-before-HUD, 71 original sprites, zero missing/unsupported assets, and identical active simulation endpoint player `(1368.99,3107.3)`, five orbs, health 25, grounded. Logs, BMPs and build log are in the packet.

The first baseline and hide37 are **not a causal pair**: their non-ID37 crop `[50,100,450,500)` differs at 127,716 RGB pixels due to the wall-clock HUD film phase. The baseline repeat and hide37 have **zero** differences in that crop and above y590. Only that registered pair is used below. BMP SHA-256: baseline repeat `42d10ba7b759ed9bcb15747317fde0f2863189814bdfa90f41f182f773035c9a`, hide37 `01928cb667cb81001fbbfe8be163269a677cbacceb48ba32234a88e0c0191700`; first, phase-unmatched baseline `b0aaac607befdc492d147078123319e5c2eef4b8e4c4a63becb6857d3d92f457`.

| Registered baseline repeat → hide ID37 | Observation |
| --- | --- |
| RGB changed | 51,801 pixels |
| Half-open changed bbox | `[0,596,1269,687)` |
| Changed above y590 | 0 pixels |
| At (100,670) | `(3,2,2)` → `(56,48,34)` |
| At (640,686) | `(14,11,8)` → `(64,55,40)` |
| At (640,590) | unchanged `(98,89,72)` |

## Source and original-reference gate

Forest XML `Game/XmlFiles/levels/forest_exploration_1.xml:612-635` authors ID37 as `kMiddleForeground`, 2048×512 `shrubbery.png`, alpha `.95`, repeat-X, follow-camera-X, UV coefficient `.000690`, fixed Y, and **no player-occlusion fade**. XML SHA-256 `6d36a837708a732ac7d2a085010a3d22229e29618b8594b5c2576d491c4161c5`; PNG SHA-256 `5a834d342975883ca54b79aa2da08730e86db11aee282f6a7a094f1fec05c412`. Original `ParallaxLayer.cpp:26-100` follows camera and shifts a one-span UV; `Sprite.cpp:626-672`, `BasicLightTexture.fx:53-59`, and `Graphics.cpp:392-405` use authored alpha/light texture and source-over. Current `LevelScene.cpp:1329-1405` retains depth-interleaved painter order, follow/UV and alpha. The prior single-alpha fix is already present. Known small follow-lag and mip-filter residuals have separate gates; this capture does not show either as a **broad** lower-darkness cause, nor does it identify a new first divergent field.

Saved Steam Forest screenshot `../store_media/screenshots/14.jpg` (1920×1080 JPEG) is a qualitative Forest-1 landmark/palette reference only. Its camera/player/orb state, original render ordinal, effect/film phase, sampler and backbuffer state are not matched to this 1280×720 frame or the previously delivered 10-second knife/FPS MP4. It cannot adjudicate ID37 edge, darkness or exact Windows parity. The next justified correction gate is a lossless same-render-ordinal original D3D10 ID37 draw/frame packet with camera, ordered draw, UV, alpha/light, bound mip/sampler and pre/post-ID37 target, compared to a separately matched Linux state; patch only a measured divergent field. No such original runtime/packet is available here. Jerome's visual review and overall parity remain open.

## Release

After all three captures, `docker ps` showed no Death Leak container and process scan showed no game or X11 recorder. Exclusive physical Xorg `:0`/NVE7 GPU and Docker build/capture lane **released**. Only this unique report and ignored evidence packet were written; existing dirty files were preserved. No production fix, commit or push was warranted.
