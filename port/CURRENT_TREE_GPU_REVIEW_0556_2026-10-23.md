# Current dirty-tree physical-GPU gameplay review — 2026-10-23 05:56 handoff

## Scope and preflight

The checkout is on `feature/linux-renderer-foundation` at HEAD `19c28af`, with shared dirty source, test and report changes preserved. I read `PORT_WORK_STATUS.md` and `port/AIM_LINE_MAIN_SCOPE_FIX_2026-10-23.md`. The latter's forced Docker production build and focused aim-line/strafe CPU tests passed on the frozen dirty tree, with executable SHA-256 `4011c2fc6de7a758b46331b9c8bffcf49da933743806f8572f334dcdb318e158`. Its full integration **failed** at `village_agri`: expected `orbs=187 health=1 balance=187`, actual `orbs=188 health=4 balance=188`. I did not rebuild, alter source, alter the integration fixture, commit, or push.

Before touching the GPU, `ps` showed physical Xorg `:0` and no Death Leak game, FFmpeg, compiler or build process; `docker ps` showed only the unrelated mobile-games service and OpenClaw sandbox. The frozen executable hash still matched after capture. The GPU/build lane was used exclusively for this run and is released now: the recorder exited, and a final process check found no Death Leak game or FFmpeg process.

## Fresh artifact and verification

- Video: `port/build/current-tree-gpu-review-0556-2026-10-23/forest-1-current-tree-active-10s.mp4` (8,632,837 bytes), SHA-256 `f83f852c766608cd0f95f218953269a49921a778556a1e9fa34034298aa56fad`.
- The ignored evidence directory contains `record.sh`, `game.log`, `window.txt`, `ffmpeg.log`, `ffprobe.txt`, `decode.log`, and `still-{1,5,9}s.png`.
- Recorder waited for both the named game window and the authored forest-1 level-entry log before X11 window-ID capture. It ran on physical Xorg `:0` with `/dev/dri` using `deathleak-capture:post-foliage` and the existing LightDM Xauthority bind. Scripted forest-1 input was `B:300,F:99700`, fixed simulation step `0.0166667`; audio used the dummy driver and the MP4 is video-only.
- `game.log`: `video_driver=x11`, OpenGL `accelerated=1 software=0 mode=strict-accelerated`, `nouveau / NVE7`, GPU `Sepia.fx`, 71 original level sprites, zero missing/unsupported assets, authored forest ambience and level entry.
- `ffprobe.txt`: H.264, 1280×720, 30 fps, **300 decoded/readable frames**, 10.000000 seconds. Independent complete `ffmpeg -xerror` decode exited 0 with empty `decode.log`.

## Visual review and limit

The 1/5/9-second stills all show active forest-1 gameplay: player, HUD, moving scenery, bridge/trunks, foliage and pickups; orb HUD changes 2→5→5. No menu, desktop, black frame, obvious missing layer, or setup overlay appears in these samples. A right-pointing aim line is visible, but this clip does not validate opposed-stick strafe or aim-angle correctness. The lower foreground stays notably dark and heavy, especially at 5/9 seconds, while the lower background/ground haze appears subdued; at 9 seconds a bright pale region at right has a fairly sharp horizontal edge. This is consistent with the previously nominated forest lower-atmosphere fidelity concern. It is **not** a proven renderer defect or matched-Windows parity failure: the available original still is not registered to this camera, level time, effect state, viewport and player state. No speculative visual source change was made.

## Delivery and outstanding gate

A private Discord MP4 attachment to Jerome was **not sent**. This child session exposes no Discord/message-send or file-attachment tool (searched the available tool catalog); `view_image` only loads local images privately and cannot deliver files. The parent/originating session must use an authorized attachment-capable message tool, if available, with the verified MP4 path above. The file is local and ready. This run does not claim Jerome's human visual acceptance, matched original-Windows parity, passing full integration, or a clean committed tree.
