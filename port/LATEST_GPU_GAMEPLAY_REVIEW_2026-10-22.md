# Latest pushed-HEAD physical-GPU gameplay review — 2026-10-22

## Provenance and capture

Branch `feature/linux-renderer-foundation`, local HEAD and remote branch both `50ba9e923a3e75f80f7eb69c11d4158bd332f339` (weather rain particle spray rendering). `deathleak-linux-build:bookworm make -C port -j2` passed with the existing binary up to date; its 14:57:34 PDT mtime postdates the 14:52:53 PDT commit. Process inspection before capture found physical Xorg `:0` but no game, FFmpeg, or competing build. Existing current-gameplay footage was from older `d4c6b30`, so a fresh clip was required.

Fresh video: `port/build/latest-gpu-gameplay-review-2026-10-22/forest-1-head-50ba9e9-active-10s.mp4`; SHA-256 `436a40893eb35d02b19f946b30d8d82313e8b46716111e051b240017b1691c24`; 10,010,264 bytes. The ignored evidence directory also contains `record.sh`, `game.log`, `window.txt`, `ffmpeg.log`, `ffprobe.txt`, `decode.log`, and `still-{1,5,9}s.png`. The script starts the authored forest-1 level with `B:300,F:99700`, waits for both the game window and forest-1 entry log before recording, and uses X11 window-ID capture. Reproduce from repo root with:

```sh
docker run --rm --hostname jerome-Linux-MacBookPro --device /dev/dri \
  --mount "type=bind,src=$PWD,dst=/src" \
  --mount type=bind,src=/tmp/.X11-unix,dst=/tmp/.X11-unix \
  --mount type=bind,src=/tmp/deathleak-current-xauth,dst=/tmp/deathleak.Xauthority,readonly \
  -e DISPLAY=:0 -e XAUTHORITY=/tmp/deathleak.Xauthority -w /src \
  deathleak-capture:post-foliage \
  sh port/build/latest-gpu-gameplay-review-2026-10-22/record.sh
```

The host's current session authority file and matching Xauthority hostname are necessary here; the earlier LightDM authority-file attempt and a mismatched-container-hostname retry did not authenticate. No cookie was copied into the repo or logged. `game.log` confirms `video_driver=x11`, OpenGL accelerated=1/software=0/strict mode, `nouveau / NVE7`, GPU `Sepia.fx`, 71 authored sprites with zero missing/unsupported assets, and forest-1 ambience/entry; it logs no level transition. Recording used dummy audio and is video-only. Recorder and game exited, leaving the GPU lane free.

`ffprobe.txt` verifies H.264, 1280×720, 30 fps, **300** readable frames, **10.000000 s**. A separate full FFmpeg decode in `deathleak-capture:post-foliage` exited 0 with an empty `decode.log`. The FFmpeg X11 shared-memory/probe warnings were nonfatal. At 1/5/9 seconds all stills show active forest gameplay with HUD and player, not menu, setup, or lock overlay; camera/scenery advance and orb HUD goes 3→5→5.

## Visual findings and acceptance limit

The clip shows layered trunks and foliage, sloped raft sections, pickups, the player/knife, and responsive camera progression without an obvious missing layer, black frame, tearing, or overlay. Compared qualitatively with original Steam `store_media/screenshots/14.jpg`, scene content and sepia palette are recognizable. However, the 5/9-second frames show a conspicuously flat-topped pale shaft at right and dark foreground vegetation/ground; the Steam reference looks softer/brighter at its lower atmosphere. These are observations, **not** a proven new renderer regression: the reference has different camera, player, health/orb, effect state, and letterboxing. Prior ID37/ID15 audits cover those authored contributors. This 10-second opening clip does not reach the later rain/flash overlap state, which has separate physical-GPU A/B evidence in `port/LATER_RAIN_FLASH_GPU_2026-10-22.md`. No speculative renderer edit was made.

**Ready for local review/delivery:** the verified MP4 above is the current pushed-HEAD active-core clip. It has not been externally attached or accepted by a human. It proves this bounded physical-Xorg/NVE7 visual sample, not pixel parity with Windows, complete port gameplay, audio quality, or overall performance. A matched original Windows frame (camera/viewport/state/time) remains the useful parity gate. Unrelated working-tree changes were untouched.
