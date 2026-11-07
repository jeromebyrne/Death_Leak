# Waterfall-bottom ID67 center-left platform — atomic NVE7 pixel gate

## Verdict

**Positive Linux pixel contribution and post-scene visibility; no original/port parity verdict.** At active Waterfall-bottom render ordinal 826, XML ID67/scene ordinal 6 changes 7,881 RGB pixels immediately across its draw, wholly within x=358..506, y=242..363. Same-executable visible/hidden comparison after later scene draws and GPU Sepia, before HUD, changes 7,632 pixels, wholly within x=359..506, y=244..363. Left, right and player controls remain byte-identical at both boundaries. This makes the center-left platform materially visible, not merely submitted. No matched original D3D10 active-bottom pre/post image or command exists, so no production fix or original/Linux parity claim follows.

## Selection, provenance and state

Selected the previously read-only ID67 (`PRIVATE_WATCHDOG_BOTTOM_ID67_CENTER_PLATFORM_READONLY_20261024T0410Z.md`) because its source-authored on-screen box `[355.77,237.63,509.13,366.81)` and nonzero asset alpha make it eligible; ID6, ID10, ID23, and the distinct upper-right same-art ID69 already have atomic gates. This is **not** the Forest ID67 foliage.

Ignored isolated evidence: `port/build/private-bottom-id67-atomic-gpu-20261024t0422z/`. The overlay is copied from the ID6 opt-in diagnostic and only targets texture `Game/Media/objects/floating_platform_1.png` at zero-based scene ordinal 6; it saves full-frame 32-bit BMPs immediately before/after that command and conditionally omits exactly that draw when `DEATHLEAK_PRIVATE_BOTTOM_ID67_HIDE=1`. Shared `linux/main.cpp`, renderer, tests, Sea-2 and production assets were untouched. Overlay SHA-256 `491e3ca25180ffd42070e12f8eeb1de1dae2f5a9000d5c05daab5cb4e77eb6f8`; built executable SHA-256 `9dbc44f9a3010877e6ae2499ef9176ad053ca7652a741b9b95ec4e736a55bdfd`; isolated Docker compilation exited 0 (`build.log`).

Serial runs used physical Xorg `:0`/`/dev/dri`, `deathleak-capture:post-foliage`, X11, `DRI_PRIME=pci-0000_01_00_0`, nouveau override and the same executable. Logs confirm strict accelerated OpenGL `vendor=nouveau renderer=NVE7`, initialized GPU Sepia and exit 0. Both ran `--play Game/XmlFiles/levels/forest_exploration_1.xml --script B:1800 --frames 827 --fixed-dt 0.016666667 --capture /out/nve7-{visible|hidden}/final.bmp --atomic-postfade-waterfall-packet /out/nve7-{visible|hidden}/packet.json`; both set their corresponding `DEATHLEAK_PRIVATE_BOTTOM_ID67_PREFIX`, and **only** the hidden arm set the hide variable. A first visible probe using stale X authority failed before renderer initialization and generated no admissible pixel evidence; the successful pair used the current authority file.

Both packets agree on active `waterfall_bottom.xml`, update/render ordinal 826, effective dt 1/60, player `(833.383118,1370.51953)`, camera draw/stored `(438.31134,1445.68396)`, zoom 1, and all 35 ordered scene command dictionaries. Target ordinal 6 has destination `[355.77063,237.627304,153.357437,129.179474]`, alpha 153/opacity .6, rotation 0, `flipX=false`, `flipY=true`, no repeat, UV shift or effect displacement. The diagnostic pins camera at frame 826 to the saved Linux witness; this is a disclosed intervention, not an original-game natural match. The two immediate pre-ID67 BMPs are byte-identical (SHA-256 `29fde9b8fc612a3df80166287939bd4e74313dc01d5a27bda400a26287e39fa9`), as is the hidden post-ID67 BMP. Visible post-ID67 hash is `1d18146d8e1d757defcda54dd194cd3a119e353338013f71d052dc849f150acc`.

## Lossless RGB result

Changed means any nonzero RGB channel; mean absolute RGB is on 0–255. Pillow comparisons of saved 1280×720 BMPs:

| Boundary / region | Changed pixels | Mean abs RGB |
|---|---:|---:|
| Visible immediate pre→post ID67, full frame | 7,881 / 921,600 | 0.57251 |
| Immediate pre→post, platform `[355,237,510,368)` | 7,881 / 20,305 | 25.98486 |
| Visible→hidden post-Sepia/pre-HUD, full frame | 7,632 / 921,600 | 0.05031 |
| Post-Sepia/pre-HUD, platform | 7,632 / 20,305 | 2.28358 |
| Left `[0,0,300,720)`, right `[900,0,1280,220)`, player `[842,338,964,482)` controls at both boundaries | 0 each | 0 |

Final post-overlay/pre-EndFrame frames differ outside the platform (including controls), so they are **not** used as a causal estimator: unpinned film phase varies across serial runs. The clean pre-HUD stage establishes survival through later scene composition and Sepia. The next parity discriminator is a provenance-qualified original D3D10 active-bottom ID67 command and same-render pre/post plus final-scene readback at matched camera/player/viewport/clock. No source-proven divergence, software edit, commit or push. Existing dirty files preserved; `PORT_WORK_STATUS.md` untouched. Capture/build containers exited; **exclusive physical GPU/build lane released**.
