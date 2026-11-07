# Waterfall-bottom ID6 upper cave backdrop — atomic NVE7 GPU pixel gate

## Verdict

**Positive Linux pixel contribution, attenuated but surviving later composition; no original/port parity verdict.** At active Waterfall-bottom render ordinal 826, XML Sprite ID6/scene ordinal 0 changes 303,933 RGB pixels immediately across its draw. Same-binary visible/hidden comparison changes 120,716 pixels after later scene draws and Sepia but before HUD, confined to ID6's screen extent; an unaffected right and lower control remain byte-identical. Thus ID6 is materially visible in the Linux composition, not merely submitted. The original active-bottom D3D10 command and immediate pre/post ID6 image remain unavailable; no source-proven defect or production fix follows.

## Reproduction and identity

Preflight found no competing Death Leak build/capture container or process on physical Xorg `:0`; the resident Xorg server itself was active. Evidence is isolated under ignored `port/build/private-bottom-id6-atomic-gpu-20261024t0412z/`. `main.cpp` is a copied ID69 opt-in diagnostic overlay altered to gate only texture `Game/Media/levels/underwater_cave/cave_background.png` at scene draw count 1/ordinal 0; it saves BGRA32 BMPs immediately before and after the draw and conditionally skips only that draw when `DEATHLEAK_PRIVATE_BOTTOM_ID6_HIDE` exists. No shared renderer, main source, tests, Sea-2, or production code was edited. Overlay SHA-256 `ba50cb54729d20a0ebd513cf6ad5aa37eca9cdbc1d4fdeb4542e83ca320fda8d`; isolated executable SHA-256 `8d6995abea2b725cae0312b0467d29ef26a9e25b2fcdec53a741a47f9d5c19a5`. `deathleak-linux-build:bookworm` compilation succeeded (`build.log`).

Serial same-executable visible and hidden runs used physical Xorg `:0`, `/dev/dri`, `deathleak-capture:post-foliage`, `SDL_VIDEODRIVER=x11`, `DRI_PRIME=pci-0000_01_00_0`, `MESA_LOADER_DRIVER_OVERRIDE=nouveau`; both logs confirm strict accelerated OpenGL `vendor=nouveau renderer=NVE7`, GPU Sepia.fx initialized, and exit 0. Each used `--play Game/XmlFiles/levels/forest_exploration_1.xml --script B:1800 --frames 827 --fixed-dt 0.016666667 --capture /out/nve7-{visible|hidden}/final.bmp --atomic-postfade-waterfall-packet /out/nve7-{visible|hidden}/packet.json`. The hidden arm had `DEATHLEAK_PRIVATE_BOTTOM_ID6_HIDE=1`; the valid visible arm had that variable **absent**. An initial accidentally hidden nominal-visible run (empty but present hide variable) was overwritten by the valid visible rerun and is excluded from the measurement.

Packets agree on active `waterfall_bottom.xml`, player `(833.383118,1370.51953)`, frozen draw/stored camera `(438.31134,1445.68396)`, and all 35 ordered scene commands; ordinal 0 exactly equals the saved `private-bottom-fog-packet-20261024t0320z/packet.json`: destination `[-172.829773,-484.426453,1384.80017,1038.99573]`, white alpha 128/opacity .5, rotation 0, flipX false/flipY true, no repeat/UV shift. The overlay pins render/stored camera only on frame 826 because replay camera depends on wall-clock duration; this is a disclosed Linux intervention, **not** a natural matched original-game frame. The original XML/asset/Sprite shader contract and lack of original comparator are documented in `port/PRIVATE_WATCHDOG_BOTTOM_ID6_UPPER_CAVE_BACKDROP_SOURCE_GATE_20261024T0400Z.md`.

## Lossless RGB comparison

Changed means any nonzero RGB channel, mean absolute RGB on 0–255; Pillow comparison of the saved 1280×720 BGRA32 BMPs. ID6 projected extent is conservatively `[0,0,1213,556)`; right control `[1220,0,1280,720)`, lower control `[0,560,1200,720)`. The pre-ID6 full-frame images are byte-identical (SHA-256 `e26ef58a562d45a8f8bf40aec887e913d93ac5b82bb5bfc3223ad44672d38998`); hidden post-ID6 equals that same hash. Visible post-ID6 is `527216d379aab35e9bb779b9155ff25dcb476ae2b216d476f853ea40ed213699`.

| Boundary / region | Changed RGB pixels | Mean abs RGB |
|---|---:|---:|
| Immediate visible pre→post ID6, full frame | 303,933 / 921,600 | 1.86833 |
| Immediate pre→post, ID6 extent | 303,933 / 674,428 | 2.55305 |
| Immediate pre→post, right and lower controls | 0 each | 0 |
| Visible→hidden post-Sepia/pre-HUD, full frame | 120,716 / 921,600 | 0.16881 |
| Visible→hidden pre-HUD, ID6 extent | 120,716 / 674,428 | 0.23068 |
| Visible→hidden pre-HUD, right and lower controls | 0 each | 0 |
| Visible→hidden final post-overlay/pre-EndFrame, full frame | 100,719 / 921,600 | 0.11262 |
| Visible→hidden final, right and lower controls | 0 each | 0 |

Pre-HUD hashes differ (`7ac8644e4e9d788967e72871fdd2dbe06c4d9004e7bcaf9073818dadea361ae0` visible; `1303bb78a4e21f8d1e22d622546d4177eaf0e328d563d81f5665a09a22d7ff60` hidden). The clean pre-HUD control is the reliable post-scene visibility measurement. This pair also has zero residual in the chosen final-frame controls, but film phase is not pinned and final-frame deltas are only corroboration, not the principal estimator.

The next parity discriminator is a provenance-qualified original D3D10 active Waterfall-bottom ID6 draw at registered camera/player/viewport with texture/UV, transform, sampler/mip/blend and immediate pre/post plus final-scene lossless readbacks. No original-versus-Linux pixel equivalence is claimed. `CHECKPOINT_STATUS.md` was already dirty under other ownership, so no append was safe. No commit/push. Both capture containers and the isolated build container exited; **exclusive physical GPU/build lane released**. `PORT_WORK_STATUS.md` untouched.
