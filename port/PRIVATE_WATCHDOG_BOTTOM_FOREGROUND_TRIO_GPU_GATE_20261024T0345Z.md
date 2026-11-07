# Waterfall-bottom foreground trio — same-render physical GPU gate (2026-10-24 03:45Z)

## Verdict

**Positive, localized Linux pixel effect; no original parity verdict or production visual change.** At active Waterfall-bottom scene draw ordinal 826, foreground XML IDs 13–15 (renderer ordinals 19–21) alter **15,125/17,568 pixels** of the frozen player-body bounding box `[842,338,964,482)` between immediately-before-ID13 and immediately-after-ID15 scene-target readbacks. Mean absolute RGB change there is **34.099/255**, maximum channel change 88. The unaffected left control `[0,0,200,720)` changes **zero** pixels. These are sequential same-render readbacks **before** later cave blur, ID25, fog, sepia, film and HUD. They prove the trio materially veils/modifies the player region in this Linux draw; they do not prove the original D3D10 game would differ.

## Reproduction and identity

Ignored packet directory: `port/build/private-bottom-trio-gpu-20261024t0345z/`. Its `main.cpp` is a copied, isolated opt-in diagnostic overlay of the prior bottom-fog packet, not a production source edit. It records BGRA32 BMPs immediately pre/post each of ordinal 19/20/21 via `Renderer::SaveFrame`; adjacent post/pre BMPs byte-match. It gates readbacks on `Game/Media/waterfall_4.png` **and** ordinal 19–21. Because the unmodified fixed-dt replay reproduces player/UV but its lagged camera varies with wall-clock run time, the overlay pins only the final frame's draw camera and reported camera to the frozen packet `(438.31134,1445.68396)` at ordinal 826. This is a diagnostic intervention, not a claimed natural exact replay. The original unpinned replay is retained under `unmatched-camera/`.

Rebuilt in `deathleak-linux-build:bookworm` from read-only current source and `/out/main.cpp`; binary SHA-256 `21ba88bc733870d4bdc728f288bafcc2158505b3f46b96ea4970ed686733aa6d`, overlay SHA-256 `57e393b6cd541064253cbd68008202908fd0183b282f5dda6949c06e4a5bd1e8`. Serial capture used `deathleak-capture:post-foliage`, host X11 `:0`/LightDM Xauthority, `/dev/dri`, `SDL_VIDEODRIVER=x11`, `SDL_RENDER_DRIVER=opengl`, strict accelerated renderer, physical nouveau **NVE7**, initialized GPU Sepia.fx, and this invocation:

`--play Game/XmlFiles/levels/forest_exploration_1.xml --script B:1800 --frames 827 --fixed-dt 0.016666667 --capture /out/final.bmp --atomic-postfade-waterfall-packet /out/packet.json`

With `DEATHLEAK_PRIVATE_BOTTOM_TRIO_PREFIX=/out/trio`; build and run exited 0. `packet.json` SHA-256 `31e4a71da13d5c1571f81ab43497f49a8cedb48ef75e8628456f8c031d20a522`. The new packet's player `(833.383118,1370.51953)`, body ordinal 17, camera, XML-13–15 ordinal 19–21 destination/UV/full command dictionaries, and following ordinal 22 **exactly equal** those of frozen `port/build/private-bottom-fog-packet-20261024t0320z/packet.json`. UV V shifts are `0.469333529`, `0.384000212`, `0.17066668`; each trio draw has opacity 0.2 and tint alpha 51. Other procedural state is not asserted identical; the camera pin is disclosed above.

## Same-render lossless measurements

All counts compare RGB pixels of `trio.draw-N.pre.bmp` to `.post.bmp` at the pre-sepia scene target; the group compares `trio.draw-19.pre.bmp` to `trio.draw-21.post.bmp`. Changed means any nonzero RGB-channel difference, with no tolerance. The player box is the frozen ordinal-17 destination rounded to a containing integer rectangle; control is outside the waterfall rectangles. BMPs are 1280×720 BGRA32. `trio.draw-19.pre.bmp` SHA-256 `597e72fc8dcb116e72685d679a391a05868b95f9db6f3ac35d2`; `trio.draw-21.post.bmp` SHA-256 `64c35965acc2fb6065a302829f1f30ef61c5e0984589a45fe802708bf8548df5`.

| Draw boundary | Player `[842,338,964,482)` changed / 17,568 | Player mean absolute RGB | Center `[300,100,1000,600)` changed / 350,000 | Left control changed / 144,000 |
|---|---:|---:|---:|---:|
| ID13 / ordinal 19 | 15,125 | 18.761 | 323,567 | 0 |
| ID14 / ordinal 20 | 7,879 | 4.069 | 298,120 | 0 |
| ID15 / ordinal 21 | 13,380 | 11.269 | 317,620 | 0 |
| **Whole trio pre-19 → post-21** | **15,125** | **34.099** | **323,567** | **0** |

Group total is 503,978 changed RGB pixels across the full 921,600-pixel scene target. The group count is not the sum of individual counts; the same pixels are composited repeatedly. `post-19 == pre-20` and `post-20 == pre-21` byte-for-byte, proving there is no intervening submission in this recorded boundary. Later draws can change final visibility, so this is not a claim that the final authored picture changes by the same amount.

## Disposition

The foreground trio is an active, player-region-affecting Linux layer, not a dead/offscreen candidate. The available original reference remains unmatched Waterfall-top marketing imagery, not an ordinal-826 bottom D3D10 scene-target packet. Therefore opacity, phase, order and art remain authored; no original-versus-port first divergent field is established. No production source, XML, tests, Sea-2 work, or `PORT_WORK_STATUS.md` changed; no commit/push. The exact next parity gate is a matched original D3D10 pre-ID13/post-ID15 target plus command fields at this camera/player/UV phase, then registered residual against these readbacks. Physical GPU/build lane released; no capture container remains.
