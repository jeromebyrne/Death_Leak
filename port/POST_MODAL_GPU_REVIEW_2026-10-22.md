# Post-modal physical GPU review — 2026-10-22

## Verdict

**Focused modal-clock behavior passes; physical accelerated gameplay-to-modal capture passes.** The forest clip is `port/build/post-modal-gpu-review-2026-10-22/forest-1-post-modal-active-12s.mp4` (SHA-256 `230ab4e225004e8c0f5ffcc54c012f73d348e34dbedbc6f87dd8f0f0b78981a6`). This is a Linux GPU/CPU regression check, not a matched Windows pixel-parity result. No software change was indicated or made.

## Revision and build provenance

The task began at `c706098` (`Freeze object visual clocks during Linux modals`). While this lane was running, other work advanced the shared branch through `4037c23` to `0cdce25`. `git diff c706098..0cdce25 -- port/game/LevelScene.cpp port/game/LevelScene.h port/linux/main.cpp` is empty: the exact modal-clock implementation under review did not change. The forced production Docker build (`deathleak-linux-build:bookworm`, `make -C port -B -j2`) passed; `build.log`/`rebuild-production.log` show only existing TinyXML warnings. Because the shared output executable was overwritten by concurrent builds, the capture is accurately described as **a descendant-HEAD build containing the unchanged c706098 modal fix**, not an immutable c706098 binary proof. The final observed executable SHA-256 was `9205be4ab7e1dedf0a83770f712b2c300d8106b341d3f81a05d8c480e30be5ba`.

## Focused behavior gate

The reproducible C++ harness embedded in `port/FOREST_MODAL_VISUAL_CLOCK_FIX_2026-10-22.md` compiled and passed twice from the repository root. For actual `forest_exploration_1.xml` ScrollingSprites 119/122, 120 paused 1/60-s calls kept drawn destination XY, opacity and tint alpha unchanged; the first unpaused call advanced XY/fade exactly one tick. In a separate process, actual `waterfall_top.xml` parallax ID6 advanced UV Y by `120 * 7 * (2/3) * 0.001 = 0.56` over those same 120 paused calls. This explicitly verifies frozen object clocks **and** live authored parallax scroll, using a mock renderer that inspects draw parameters before postprocess.

## Physical Xorg :0 evidence

The isolated `record.sh` under the ignored evidence directory launched directly into forest-1 with scripted movement and interaction (`R:130,E:25,I:31,.:99700`). It waited for the named X11 window **and** the forest authored-ambience level-entry log before recording that game window, so the clip does not begin on a menu or setup screen. The log reports SDL `video_driver=x11 name=opengl flags=14 accelerated=1 software=0 vsync=1 mode=strict-accelerated`, OpenGL `nouveau / NVE7`, initialized GPU `Sepia.fx`, 71 original sprites, zero missing/unsupported assets and `Opened kDeflection upgrade modal at frame 184 cost=375 balance=4`.

FFprobe reports H.264 1280×720 at 30 fps, **360 readable frames / 12.000000 seconds**, size 5,695,402 bytes. Full FFmpeg `-xerror` decode passed with empty `decode.log`. Inspected `still-1s.png`: player moving on the authored forest bridge, HUD/orbs and parallax scenery visible—active gameplay after entry. Inspected `still-5s.png` and `still-10s.png`: the same forest view behind the Deflection upgrade modal, with unchanged 4-orb balance and player/camera pose; no gross visual jump. The stills alone cannot isolate fog coordinates because the modal darkens the scene and postprocess shader time remains live; the CPU draw-parameter gate above is the precise fog-freeze proof. The x11grab shared-memory warning was nonfatal; capture fell back and decoded cleanly. Dummy audio was used; the clip has no audio stream.

A separate bounded direct physical-GPU frame `waterfall-r90.bmp` (PNG review copy `waterfall-r90.png`, SHA-256 `54a1cc97a795f6512888146937e1e67f286b8ed2abb23f1e6f61e3495ff21c97`) entered `waterfall_top.xml`, loaded 81 original sprites with zero missing assets, logged 3 authored ambience streams, and ended at player `(1510.99,3009.04)`, 5 orbs, grounded. The PNG visibly shows in-level gameplay, not a menu. It establishes the waterfall visual path works on the accelerated renderer; **paused UV motion itself is established by the focused CPU harness**, not by this one still.

## Failure/caveat record

The first production capture attempt exited before the level-entry gate, and two brief probes returned exit 139 after GPU initialization. An ASan diagnostic build ran 10 physical-GPU frames cleanly, then a clean forced production rebuild also ran 10 frames cleanly. The subsequent 12-s modal clip and 90-frame waterfall capture completed normally. The shared branch/binary was being modified by another lane during this interval, so the early exit's root cause is not isolated; it must not be called a proven source regression or silently erased. Production build was restored after ASan. No game/FFmpeg process from this lane remained after its captures; another independent GPU lane started afterward. No Combat file, shared status file, authored XML/art, private Discord route, or tracked software file was changed by this review.
