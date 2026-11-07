# Private current-HEAD waterfall-top physical-GPU visual gate — 2026-10-23 21:51Z

## Verdict

**Active-gameplay visual capture succeeded; negative defect gate.** Current HEAD `29984eb21b489e59c944e021b14204f8d10d3446` renders waterfall-top on physical Xorg `:0` / nouveau NVE7 in strict accelerated mode. The 120th scripted in-level frame and a separate ten-second in-level gameplay clip show the broad central dark waterfall/shaft, opposing raft ends, right ledge/orbs, lower foreground foliage and HUD; the clip's 1/5/9-second stills show the camera/player progressing right and the orb count increasing from 1 to 11. No missing asset or black render failure is present. The central curtain/character-readability contrast against store screenshot 16 is **not** a source-proven Windows-versus-Linux defect: it is a different camera, player/orb state, effect phase and processed JPEG. No renderer, XML or art change is nominated.

## Reproducible current capture

Before build/capture, `ps` showed no Death Leak game, FFmpeg x11grab recorder or competing Docker build; `docker ps` showed only unrelated persistent containers. Production binary was built serially from the current shared tree (including already-dirty `port/linux/SDLRenderer.cpp`, untouched here) into unique ignored `port/build/private-current-head-waterfall-gpu-20261023t2151z/` with:

```sh
docker run --rm --network host --mount "type=bind,src=$PWD,dst=/src,readonly" --mount "type=bind,src=$PWD/port/build/private-current-head-waterfall-gpu-20261023t2151z,dst=/out" -w /src deathleak-linux-build:bookworm make -e -C port TARGET=/out/deathleak-linux -j2
```

`build.log` records exit 0. Binary SHA-256: `4b9d1481186d30ed09c0b618d4332024362584611527131dbab5b9281b339441` (same bytes as the immediately preceding HEAD Running-rate build). The exact lossless frame command was:

```sh
docker run --rm --network host --hostname jerome-Linux-MacBookPro --device /dev/dri --mount "type=bind,src=$PWD,dst=/src,readonly" --mount "type=bind,src=$PWD/port/build/private-current-head-waterfall-gpu-20261023t2151z,dst=/out" --mount type=bind,src=/tmp/.X11-unix,dst=/tmp/.X11-unix --mount type=bind,src=/var/run/lightdm/root/:0,dst=/tmp/deathleak.Xauthority,readonly -e DISPLAY=:0 -e XAUTHORITY=/tmp/deathleak.Xauthority -e SDL_VIDEODRIVER=x11 -e SDL_RENDER_DRIVER=opengl -e SDL_AUDIODRIVER=dummy -w /src deathleak-linux-build:bookworm /out/deathleak-linux --play Game/XmlFiles/levels/waterfall_top.xml --script '.:120' --frames 120 --fixed-dt 0.016666667 --capture /out/waterfall-120.bmp
```

`frame-120.log` reports `video_driver=x11 name=opengl accelerated=1 software=0 mode=strict-accelerated`, `OpenGL device vendor=nouveau renderer=NVE7`, GPU Sepia before HUD, waterfall-top level ambience, **81 original sprites, zero missing/unsupported**, and final player `(818.985,2933.64)`, one orb, health 25, grounded. `waterfall-120.bmp` is 1280×720, SHA-256 `aef665fdf27de0fddd2e1b3a4ed7598babbecd9615412ce489b75cc399b7f211`; `waterfall-120.png` is its view copy.

The gameplay recorder is the saved exact `record.sh` in that packet, launched serially after the still capture with:

```sh
docker run --rm --network host --hostname jerome-Linux-MacBookPro --device /dev/dri --mount "type=bind,src=$PWD,dst=/src,readonly" --mount "type=bind,src=$PWD/port/build/private-current-head-waterfall-gpu-20261023t2151z,dst=/out" --mount type=bind,src=/tmp/.X11-unix,dst=/tmp/.X11-unix --mount type=bind,src=/var/run/lightdm/root/:0,dst=/tmp/deathleak.Xauthority,readonly -e DISPLAY=:0 -e XAUTHORITY=/tmp/deathleak.Xauthority -w /src deathleak-video-capture:bookworm sh /out/record.sh
``` It runs `/out/deathleak-linux --play Game/XmlFiles/levels/waterfall_top.xml --script '.:120,R:180,L:180,.:99520' --frames 100000 --fixed-dt 0.016666667`, waits for both a named X11 game window and waterfall level-entry log, then records that window via FFmpeg x11grab. `game.log` independently confirms strict accelerated NVE7, GPU Sepia, 81 sprites/zero missing and authored ambience. The intermediate MPEG-4 is transcoded to `waterfall-active-10s.mp4`; FFprobe records H.264, 1280×720, 30 fps, **300 decoded frames**, 10.000 s, 4,134,738 bytes; full FFmpeg `-xerror` decode exited 0. Final MP4 SHA-256 `d31d823bd25150f9ab62f7b58eecbd630c865a6bdf01f3f944715cf995b7fb66`. `still-1s.png`, `still-5s.png`, `still-9s.png`, and `contact-1-5-9s.jpg` are the visual inspection packet. Wall-clock video frames are not exact simulation ordinals.

## Distinct source-backed visual boundary

At frame 120 and at 1/5 seconds, the central dark fall/shaft dominates the middle while the player is difficult to discern; at 9 seconds, after camera travel to the right, the player is visible on ground beside orbs. Store `../store_media/screenshots/16.jpg` instead shows an airborne, legible player above the central gap and differently framed raft ends, with 24 HUD orbs. That is a **qualitative composition/readability gap between unmatched states**, not evidence that the current sprite draw order or opacity is wrong. Original XML authors the raft/ledge IDs32/33/52 and upper water sheets IDs6/7/10/13/31; the existing `WATERFALL_CAMERA_X_SWEEP_2026-10-23.md` aligns those three static geometry landmarks to the screenshot at camera X=315 within 2.49 px RMS. `WATERFALL_CURTAIN_X315_DISCRIMINATOR_2026-10-23.md` causally identifies those five sheet contributors within Linux. The screenshot's camera/player/orb and UV/wobble/light/weather clocks are not matched to this capture, and its JPEG/presentation history is unknown. The existing upper-curtain luma and matte observations in `WATERFALL_X315_RESIDUAL_VISUAL_GATE_2026-10-23.md` therefore remain a **negative parity gate**, not a mandate to brighten water, reveal the player, move rafts, add bars, or edit authored assets.

A defensible next discriminator is one native original-Windows lossless same-state waterfall frame plus camera/view/player/orb, ordered sheet IDs6/7/10/13/31 UV/alpha/blend, wobble/weather/light and capture-stage trace, then an equivalent Linux strict-GPU frame. This packet does not provide that comparator; no implementation patch plan follows.

## Scope and release

This report and ignored evidence directory are the only writes. No shared checkpoint, `PORT_WORK_STATUS.md`, source, renderer, tests, index, commit, push, or unrelated dirty file was changed. Final `ps`/`docker ps` verification found no Death Leak game, recorder, build or capture container. **Exclusive physical Xorg :0 / NVE7 GPU and Docker build lane released.**
