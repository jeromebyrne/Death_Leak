# Current waterfall-top gameplay video — 2026-10-22

**Delivered artifact (local):** `port/build/current-waterfall-gameplay-video-2026-10-22/waterfall-top-current-head-10s.mp4`. SHA-256 `9818b8053b039c3082dd61e5aa6954a92abafd1eca4af16e6f1df495eff5ee12`; 4,193,823 bytes. This is an active in-level, window-only, video-only gameplay clip, not a menu/setup or locked screen. It was captured on current HEAD `68a038575b649ded1095df135b18ad4ecc4c6f08`; the executable SHA-256 was `294ccea1ba8dab0df62066405ec4ee4ad04f2843421ab8257f259a5952465979` (mtime 2026-10-22 15:52:59 PDT). No source/build edits or commit were made.

## Exact capture and verification

The exact executable recorder is `port/build/current-waterfall-gameplay-video-2026-10-22/record.sh`. From repository root it was launched with:

```sh
docker run --rm --network host --hostname jerome-Linux-MacBookPro --device /dev/dri --mount "type=bind,src=$PWD,dst=/src" --mount type=bind,src=/tmp/.X11-unix,dst=/tmp/.X11-unix --mount type=bind,src=/var/run/lightdm/root/:0,dst=/tmp/deathleak.Xauthority,readonly -e DISPLAY=:0 -e XAUTHORITY=/tmp/deathleak.Xauthority -w /src deathleak-video-capture:bookworm sh port/build/current-waterfall-gameplay-video-2026-10-22/record.sh
```

Recorder game command: `SDL_VIDEODRIVER=x11 SDL_RENDER_DRIVER=opengl SDL_AUDIODRIVER=dummy stdbuf -oL -eL ./port/build/deathleak-linux --play Game/XmlFiles/levels/waterfall_top.xml --script '.:120,R:180,L:180,.:99520' --frames 100000 --fixed-dt 0.016666667`. It waited for the game window **and** the waterfall-top authored-ambience entry log before capturing that window via FFmpeg x11grab `-window_id`, `-framerate 30`, `-t 10`, 1280×720 scale. The successful recorder produced `waterfall-top-current-head-10s-v2.mp4` using `-c:v mpeg4 -q:v 4 -pix_fmt yuv420p`; the delivered H.264 MP4 was produced from that verified file using:

```sh
ffmpeg -nostdin -hide_banner -loglevel error -xerror -i port/build/current-waterfall-gameplay-video-2026-10-22/waterfall-top-current-head-10s-v2.mp4 -an -c:v libx264 -preset medium -crf 20 -pix_fmt yuv420p -y port/build/current-waterfall-gameplay-video-2026-10-22/waterfall-top-current-head-10s.mp4
```

`final-ffprobe.txt` reports H.264, 1280×720, 30/1 fps, **300 read frames**, duration **10.000000 s**, 4,193,823 bytes. `ffmpeg -xerror` decoded the full deliverable to null successfully (`final-decode.log` empty). Review stills at 1, 5, and 9 seconds (`still-1s.png`, `still-5s.png`, `still-9s.png`) were extracted from the successful MPEG-4 capture, before the same-timeline H.264 transcode. The 5- and 9-second stills show the player at different world/camera positions; HUD/orb counts also change. The 1-second frame shows the shaft/bridge and HUD, with the player largely concealed by dark foreground. The sampled frames are rendered gameplay rather than menu or lock screen; the run log contains no level-transition line.

## GPU provenance and bounded visual findings

`game.log` reports SDL `video_driver=x11 name=opengl flags=14 accelerated=1 software=0 vsync=1 mode=strict-accelerated`, `OpenGL device vendor=nouveau renderer=NVE7`, and GPU Sepia postprocess, on physical Xorg `:0` with `/dev/dri`. It loaded 81 original level sprites, zero missing/unsupported assets, and three authored ambience voices for `waterfall_top.xml`. The foreground shaft, bridges/orbs, active HUD, and moving player are visible across the sampled clip. The waterfall area is most apparent early and mid-clip; the player/camera travels right so the shaft leaves the view toward the end. The continuous video makes the active scene reviewable, but this capture does not independently measure the eight sheet speeds or prove flicker-free flow; the prior fixed-camera GPU still analysis in `WATERFALL_AUTOSCROLL_GPU_REVIEW_2026-10-22.md` is the bounded rate evidence. No Windows parity or user visual acceptance is claimed.

The first direct libx264 recording in the same ignored directory was **rejected**: FFprobe found only 212 readable frames and FFmpeg logged invalid NAL units/full-decode failure. It is **not** the deliverable. The clean MPEG-4 retry and subsequent H.264 transcode passed 300-frame and full-decode checks. FFmpeg x11grab logged a nonfatal X11 shared-memory fallback. No game or FFmpeg process remained after capture. Existing unrelated working-tree files, `PORT_WORK_STATUS.md`, and shared checkpoint files were left untouched.
