# Private orb-icon current-HEAD physical-GPU gate — 2026-10-23

## Verdict

**PASS for the bounded source-exact HUD command and Linux physical-GPU active-Forest visual gate.** The corrected orb icon appears below the focus bar at the upper-left, upright, unclipped, and alongside the count `5` without visible overlap. This is qualitative Linux inspection, not matched-original-Windows pixel parity or Jerome's visual acceptance.

## Source and build verification

At preflight and capture, branch `feature/linux-renderer-foundation` HEAD was `2e2562e3d9a858663e4d27e978dd6495d03452ea` (`Place orb HUD icon from authored UI coordinates`). The preceding worker's `port/PRIVATE_ORB_ICON_PLACEMENT_IMPLEMENTATION_20261023T1858Z.md` and `port/game/Hud.cpp` are **already committed in that HEAD**; `Hud.cpp` has no working-tree or staged diff. The committed destination projects XML `(-900,360), 101x64` in the Y-up 1920x1080 UI to `{40,77.3333,67.3333,42.6667}` in the Y-down 1280x720 viewport. It retains alpha byte 191 and `flipY=false`.

The existing `/tmp/deathleak_orb_hud_spy_20261023.cpp` renderer spy was recompiled with current `port/game/Hud.cpp` and passed its independent rectangle, texture identity, alpha, opacity, and flip checks: `PASS orb destination 40,77.3333,67.3333,42.6667 alpha=191 flipY=0`. `git diff --check HEAD^ HEAD -- port/game/Hud.cpp` passed. Established Docker image `deathleak-linux-build:bookworm` built current tree with read-only `/src` and unique `/out`, `make -e -C port TARGET=/out/deathleak-linux -j2`, exit 0. Binary SHA-256: `5d299a00b51a5155aaf35f7a956640ec00dc1f453f4def1c525ec7453db9db33`.

## Physical-GPU frame

After confirming no competing Death Leak game, build, FFmpeg/x11grab, or capture container on Xorg `:0`, a fresh-process 180-frame `--play Game/XmlFiles/levels/forest_exploration_1.xml --script R:180 --fixed-dt 0.0166667 --capture` run used physical Xorg `:0`, `/dev/dri`, LightDM Xauthority, `SDL_VIDEODRIVER=x11`, `SDL_RENDER_DRIVER=opengl`, and dummy audio. The run exited 0. Log reports `video_driver=x11 name=opengl accelerated=1 software=0 mode=strict-accelerated`, `vendor=nouveau renderer=NVE7`, GPU Sepia postprocess, 71 loaded original Forest sprites and zero missing/unsupported assets. End state is active grounded Forest gameplay: player `(1368.99,3107.3)`, five orbs, health 25, focus 100, rightward movement. Thus the frame is not a menu, preview, or software-rendered surrogate.

Evidence in ignored `port/build/private-orb-current-head-gpu-20261023t1900z/`: lossless `forest-active-orb-current-head.bmp` (SHA-256 `5d7683d69074384294536f44965b9233123ce2d35d8582de52a8f9129a570630`), viewable PNG, `build.log`, `capture.log`, and binary. The full frame visibly places the white orb around x40–70/y85–105, with count `5` to its right and focus bar above; it is not cut by the screen edge. This current-tree binary also includes pre-existing unrelated dirty `SDLRenderer.cpp` and `ParticleSpray.h`, so this frame is an integration observation, not a clean-HEAD causal pixel A/B or proof that every renderer path matches original Windows.

## Commit, remote, and lane release

No new `Hud.cpp` staging/commit was possible or warranted: the scoped correction was already committed before this gate, and `git ls-remote --heads origin feature/linux-renderer-foundation` returned the exact local HEAD `2e2562e3d9a858663e4d27e978dd6495d03452ea`. It was therefore already pushed to the configured branch; no redundant push or unrelated commit was made. Existing unrelated dirty files, index, and shared `CHECKPOINT_STATUS.md`/`PORT_WORK_STATUS.md` were untouched. The named build and capture containers exited; final process check found no Death Leak game/build/capture process. **Exclusive physical Xorg `:0` / NVE7 GPU capture lane released.**
