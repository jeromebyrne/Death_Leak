# Footstep material spray — current-HEAD Docker/GPU review (2026-10-22)

## Scope and provenance

- Repository HEAD was `e2c5adf` before the work and remained `e2c5adf` afterward. Existing unrelated working-tree modifications and untracked files were preserved. This report is the only non-ignored project file written by this task; all generated evidence is under `port/build/footstep-qa-gpu-2026-10-22/`.
- Focused test compiled and ran in `deathleak-linux-build:bookworm`: **PASS** (`test-build.log`, `test.log`). It covers Running-frame re-entry/gating, contact, exact `water` exclusion versus `shallow_water`, and seeded particle parameters.
- Forced current-source Docker build (`make -C port -B -j2`) succeeded (`build.log`); built executable SHA-256 `4fb5d61d2e2728cec23df86027c849ca66988133ba9d029e5022f99ef560a0c9`.

## Physical GPU A/B

- Runs used physical Xorg `:0`, LightDM Xauthority, `/dev/dri`, SDL X11 OpenGL and fixed `dt=0.0166667`. Current and control logs (`ab-current-{1,2,5,12}.log`, `ab-control-{1,2,5,12}.log`) each identify `video_driver=x11 name=opengl flags=14 accelerated=1 software=0 vsync=1 mode=strict-accelerated`, OpenGL vendor `nouveau`, renderer `NVE7`; this was not llvmpipe/software rendering.
- Current binary was the forced HEAD build. For a controlled A/B only, an isolated copy of `LevelScene.cpp` (`LevelScene-no-footstep.cpp`) changed the single call `EmitRunningFootstep(sequence)` to a false-gated call; it was compiled to `deathleak-no-footstep` (`ab-control-build.log`). No production source was changed or replaced. Both binaries used the same forest level, `--script R:20`, fixed dt, and final-frame `--capture` at frames 1, 2, 5 and 12.
- Pixel comparison (`ab-current-*.bmp`, `ab-control-*.bmp`, `ab-diff-*.png`) found **zero changed pixels** at frames 1, 2 and 5. At frame 12, 433 pixels differ, confined to screen bbox `(655,527)-(678,551)` near the player's foot/bridge contact. Enlarged crops `ab-current-12-crop.png` and `ab-control-12-crop.png` show the transient material spray only in current HEAD. This supports the spray draw path on accelerated GPU with an otherwise matched frame. It does not establish Windows visual parity, every material's in-level visual appearance, or all animation timing cases.

## Current-HEAD active gameplay video

- `forest-1-footstep-current-head-10s.mp4` is a **local, not delivered** video-only H.264 artifact, SHA-256 `000b9c55735e0fc6b491adf63b9d3c1366dfdd0eda8171d9512b96add87d7f23`, 3,290,745 bytes. `ffprobe.txt` reports 1280×720, 30 fps, exactly 10.000 seconds and 300 readable frames; full `ffmpeg -xerror` decode passed (`decode.log` is empty). Source acquisition/transcode logs are `ffmpeg.log` and `transcode.log`.
- `record.sh` launched directly into `Game/XmlFiles/levels/forest_exploration_1.xml` with scripted gameplay, waited for both the named game window and logged level entry, then captured that window for 10 seconds. `game.log` confirms strict accelerated `nouveau / NVE7`, Sepia GPU postprocess, 71 loaded original sprites, zero missing assets, and authored forest ambience; `window.txt` records the X11 window. Stills at 1, 5 and 9 seconds show the player moving through forest gameplay with HUD/orbs and changing camera/position. They are in-level gameplay, not a menu, loading screen, or desktop.

## Limits / handoff

- The CPU test verifies exact `water` versus `shallow_water` semantics using a temporary XML fixture; the GPU A/B visually discriminates the shipped forest-ground running spray only. No matched Windows footage or per-material GPU A/B was available here.
- The MP4 has no game audio (`SDL_AUDIODRIVER=dummy`), and this job made no delivery or acceptance claim. No project source, shared checkpoint, config, or scheduler was edited; no commit was made.
