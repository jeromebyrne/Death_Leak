# Forest-1 ID21 overdraw: frame-180 physical-GPU discriminator — 2026-10-23

## Verdict

**The remaining x320 ID19 occlusion is authored painter overdraw by ID21 in this current Linux frame.** With ID22 already hidden, removing ID21 changes every x320 pixel in y440–500; then removing ID19 changes 60/61 pixels there (first y440, last y500; only y441 unchanged). Before ID21 is removed, the prior same-route ID22 gate measured zero x320 ID19 changes in y380–559. At x640, hiding ID21 changes zero pixels in y380–559 while the subsequent ID19 hide remains a positive control (38/61 changed in y440–500). The new same-binary ladder therefore directly exposes ID19 at the predicted x320 location only after ID21 removal. This closes the specific current-frame occluder question, **not** original-Windows parity, visual acceptance, or a renderer defect. No art/XML/renderer change is justified.

## Frozen current-tree capture and registration

- Read `FOREST_MIDDLE_BAND_17_19_CURRENT_SOURCE_GATE_2026-10-23.md`, its cited `FOREST_MIDDLE_BAND_CURRENT_FRAME_GPU_CAUSAL_2026-10-23.md`, and the ID22 GPU discriminator. Their source ordering predicts ID21 `tree_strip_dark.png` (same `kMiddleBackground` depth, later XML order) can cover ID19 at x320, with near-background ID22 on top. The original and port both use stable depth/painter order per the source gate.
- HEAD during this capture: `9ba65c3a7b8f618b8257a132a6c5e77a179cebb1`; dirty-tree snapshot before and after the isolated build in `port/build/forest-id21-overdraw-gpu-discriminator-2026-10-23/`. The then-shared executable predated current `Hud.cpp`, `Hud.h`, and `main.cpp`, so this gate built a **separate** current-tree target in `deathleak-video-capture:bookworm` without overwriting it. Frozen executable SHA-256 before/after: `ba78aa9a3b30848b869aca9ba17968f10f262e6dbf54ccccebfdfad1974f0777`.
- Checked `docker ps` and `deathleak-linux` processes before occupying the display; no competing Death Leak GPU capture/process was present. One exclusive physical-Xorg `:0` container with `/dev/dri`, LightDM Xauthority, and image `deathleak-video-capture:bookworm` ran three sequential variants on that exact executable: `DEATHLEAK_RENDER_HIDE_XML_IDS=22`, `22,21`, `22,21,19`. Each used `SDL_VIDEODRIVER=x11 SDL_RENDER_DRIVER=opengl SDL_AUDIODRIVER=dummy`, `--play Game/XmlFiles/levels/forest_exploration_1.xml --script R:180 --frames 180 --fixed-dt 0.0166667`, and a lossless 1280×720 BMP capture. Container exited; GPU lane released.
- Every log reports `video_driver=x11 name=opengl flags=14 accelerated=1 software=0 vsync=1 mode=strict-accelerated`; GPU identity `OpenGL device vendor=nouveau renderer=NVE7`; effect identity `Sepia.fx GPU postprocess initialized (scene before HUD)`. Each loaded 71 original sprites, zero missing/unsupported assets. All printed gameplay endpoint fields agree: player `(1368.99,3107.3)`, five orbs, health 25, balance 5, grounded, `vx=480`, zero fired/shots. Only wall-clock `elapsed` varies (3.60863–3.87907 s); no claim of byte-identical endpoint lines or measured shader-uniform parity.
- A read-only GDB draw breakpoint on the **same frozen executable and hide-22 route** measured final draw camera `cameraX=1248.98621`, `cameraY=3400`, player `(1368.98535,3107.30444)`. The GDB-produced hide-22 BMP is byte-identical to the primary hide-22 BMP (`6238754371a6dd6ced710bb6c4c2b40c5ea0cc551f00766cd543373c78b5092`), registering the camera to this binary/frame/effect capture. The temporary Xauthority copy was deleted.

## Lossless RGB discriminator

Coordinates are top-down. A changed pixel means any RGB channel differs; boxes are half-open `(left,top,right,bottom)`. All comparisons are **within the same frozen executable**, with fresh hide-22 as the baseline; older binaries' BMPs were not used as pixel baselines.

| Same-binary comparison | Changed RGB pixels, whole frame | Bbox | x320/y380–559 | x320/y440–500 | x640/y440–500 | x960/y440–500 |
| --- | ---: | --- | ---: | ---: | ---: | ---: |
| hide-22 → hide-22,21 | 130,734 | `(0,0,622,528)` | 122; first y380 | **61/61** | **0/61** | 0/61 |
| hide-22,21 → hide-22,21,19 | 81,905 | `(0,440,1280,567)` | **79; first y440, last y519** | **60/61** | **38/61** | 46/61 |
| hide-22 → hide-22,21,19 | 190,409 | `(0,0,1280,567)` | 122 | 61/61 | 38/61 | 46/61 |

For incremental ID19 at x320, y450 RGB is `(85,73,51) → (98,84,60)` and y460 `(114,99,71) → (96,83,58)` after ID21 is hidden. At x640, y470 is `(87,75,54) → (81,69,49)`, confirming the expected exposed-ID19 control. The only unchanged x320 pixel in y440–500 is y441; this does not alter the band-level conclusion. BMP SHA-256: hide-22 `6238754371a6dd6ced710bb6c4c2b40c5ea0cc551f00766cd543373c78b5092`; hide-22,21 `aeeedeb9dcf85af4066c441bafb1ea6773ac3b201b0948941c1d8abc5704db81`; hide-22,21,19 `097c4ad10d5bb6504ba5ef22c9865fb674d7f344d884a1a13597199d9b1f8e18`.

The unchanged x640/x960 ID21 control and restored x320 ID19 delta fit the XML painter order and ID21 PNG alpha prediction in the source gate. This is causal current-Linux compositing evidence, not a Windows-matched frame. A matched original-Windows camera/frame/viewport/effect state remains necessary for any parity or regression verdict.

## Scope

Only this report and ignored `port/build/forest-id21-overdraw-gpu-discriminator-2026-10-23/` evidence were written. No source, tests, shared checkpoints, `PORT_WORK_STATUS.md`, shared executable, commit, or push changed. The temporary GPU container and Xauthority copy are gone.
