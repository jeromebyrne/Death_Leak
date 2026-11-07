# Forest-1 ID21 pre-sepia physical-GPU control — 2026-10-23

## Decision

**The current Linux ID21 edge is an authored alpha transition in the pre-sepia scene, with the expected same-frame painter controls.** The isolated capture hook did not perturb the ordinary post-sepia frame: its hide-22 post BMP is byte-identical to a fresh uninstrumented, same-source build, and a second hide-22 pre/post pair is byte-identical to the first. The measured visible edge tapers to zero at x374 (y450), x376 (y460), and x392 (y500), aligning within about one screen pixel of the nearest-source-alpha zero boundary. This is a Linux-only diagnostic, **not** an original-Windows parity verdict or grounds for a renderer/art/XML change. The original-D3D10 matched pre-sepia packet and bound SRV/mip state remain the correction gate in `FOREST_ID21_PRESEPIA_CONTROL_CAPTURE_PLAN_2026-10-23.md`.

## Frozen build and physical-GPU route

- Evidence: ignored `port/build/forest-id21-presepia-control-2026-10-23-d2a25ad2/` contains frozen compiled source, overlay, two binaries, exact Makefile compiler output/build logs, source and binary SHA-256 manifests, capture script/manifest/logs, all lossless BMPs, pixel analysis JSON, GDB registration, and before/after status snapshots. HEAD `9ba65c3a7b8f618b8257a132a6c5e77a179cebb1`; the pre/post `git status --short` snapshots are byte-identical (386 pre-existing entries), and all frozen compiled source files still hash-identically to the live source at completion. No shared source, checkpoint, shared build, ID59 file, commit, or push was touched.
- The copied *current dirty source* was compiled twice in `deathleak-video-capture:bookworm` (image digest `sha256:889e8ea1d6152f480eb6f415d5c7a953f1aa9387a66bf1ff63461f238aae6e0c`): uninstrumented binary SHA-256 `9ff1c721345f13f546a3a0877e207a8a482ef7acfa41f00c1ea74eee8625d8ab`; overlay binary SHA-256 `d403a589841e125bc9d78a9f6249b96bf77053b32f47b09c00efbfebf9554818`. The overlay has only the exact-one-replacement capture hook immediately before `ApplyScenePostprocess`; `SaveFrame(capture + ".pre.bmp")` is synchronous, before sepia/HUD. The ordinary `--capture` remains after sepia/HUD. `Game/tinyxml` is unchanged.
- Checked `docker ps` and Death Leak processes before taking the physical Xorg `:0` lane; no competing capture owned it. Six variants ran **sequentially**, same image/source mount, `/dev/dri`, X11 socket, read-only LightDM Xauthority, and hostname matching its cookie. Each used `SDL_VIDEODRIVER=x11 SDL_RENDER_DRIVER=opengl SDL_AUDIODRIVER=dummy`, `--play Game/XmlFiles/levels/forest_exploration_1.xml --script R:180 --frames 180 --fixed-dt 0.0166667`. The initial unauthenticated hostname attempt produced no BMP and was corrected before the verified series. All successful logs identify `video_driver=x11 name=opengl flags=14 accelerated=1 software=0 vsync=1 mode=strict-accelerated`, `vendor=nouveau renderer=NVE7`, and `Sepia.fx GPU postprocess initialized (scene before HUD)`; 71 sprites loaded, zero missing/unsupported. Every image is 1280×720, top-down coordinates here.
- GDB on the **same overlay binary**, hide-22 route, counted 180 `LevelScene::Draw` calls and measured final-draw camera `(1248.98621,3400)`, player `(1368.98535,3107.30444)`. Its pre and post BMPs match the ordinary hide-22 captures byte-for-byte. Successful run endpoints agree to logged precision: player `(1368.99,3107.3)`, orbs 5, health 25, balance 5, grounded, `vx=480`, no fired/shots; only wall-clock elapsed differs. The temporary Xauthority copy was shredded. No Death Leak GPU container remained; the lane was released.

## Lossless same-frame controls

A = `hide-22`, B = `hide-22-21`, C = `hide-22-21-19`, D = `unmasked` (hide-ID environment **unset**). Counts mean changed RGB pixels; boxes are half-open `(left,top,right,bottom)`. All comparisons use new frozen-binary BMPs only, never the older BMP as a pixel baseline.

| Pre-sepia scene comparison | Changed pixels / bbox | x320 y440–500 | x640 y440–500 | x960 y440–500 | Edge ROI `(350,430,405,511)` |
| --- | ---: | ---: | ---: | ---: | ---: |
| A→B (remove ID21) | 132,384 / `(0,0,622,528)` | **61/61** | **0/61** | 0/61 | 2,192 |
| B→C (remove ID19) | 82,910 / `(0,440,1280,567)` | **61/61** | **39/61** | 47/61 | 3,255 |
| A→A repeat | 0 / none | 0 | 0 | 0 | 0 |
| D→A (remove ID22) | 318,253 / `(0,0,1275,624)` | **61/61** | 0/61 | 0/61 | — |

The opaque x320 ID21 contribution, transparent x640/no-ID21 control, then newly exposed x320/x640 ID19 contribution all pass. ID22 masks the x320 region in the unmasked composition. The post-sepia/HUD A→B and B→C counts are respectively 130,734 and 81,905; those are registration/control results, not a cross-stage edge filter test. The **uninstrumented post A vs instrumented post A is 0 changed RGB pixels, whole-frame** (SHA-256 `6238754371a6dd6ced710bb6c4c2b40c5ea0cc551f00766cd543373c78b5092a`). The A pre repeat is also exact (SHA-256 `20141bb10030a4d3e418cfdefdc9223210c24b06b048095806ab4ded26435733`). All image hashes are in `bmp-sha256.txt` and per-image results in `analysis.json`.

## Two-dimensional alpha-edge reading

The x350–404 / y430–510 ROI includes full, partial, and zero-alpha positions. The pre-sepia A→B RGB L1 distance at y450 across x369–375 is `90,96,87,51,18,5,0`; at y460 x371–377 it is `148,119,80,39,13,3,0`; at y500 x383–394 it is `95,90,75,55,33,17,8,6,2,2,0,0`. The observed final nonzero columns are x374, x376, and x392. Under the measured camera/projection and ordinary inverted V, nearest PNG alpha at those rows reaches zero at x375, x377, and x394 respectively (the last source alpha is 1 at x393). Exact RGB distance is underlay/color-dependent, not a direct alpha estimate; full per-row RGB profiles are retained in `analysis.json`.

At y440, A→B changes x320 but not x360–400 despite the source PNG's partial alpha near x367–374. This row is therefore **not** a clean edge-width control in the composed scene; nearby visible content/overdraw must be accounted for before attributing that zero delta to texture filtering. The 2D ROI and lower clean rows avoid treating the boundary as a vertical straight line. No abnormal transition is established by this Linux measurement, and no speculative fix was made.

## Boundary

A lossless original-Windows pre-sepia ID21 packet at the same camera, viewport, render ordinal, effect phase, and bound SRV `MipLevels`/range is still absent. Compare that packet against these frozen Linux controls before proposing any scoped ID21 sampling/alpha-edge treatment. Keep authored ID19→ID21→ID22 order, XML alpha, and global mip behavior unchanged meanwhile.
