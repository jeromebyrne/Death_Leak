# Current-HEAD post-rotation active gameplay capture — 2026-10-22

## Result

Fresh forest-1 ACTIVE gameplay video at HEAD `fcb713b5da9c0eaf67ab34bec63e5611e30d3eab`:

- Video: `port/build/post-rotation-current-head-gameplay-2026-10-22/forest-1-post-rotation-current-head-active-10s.mp4`
- SHA-256: `190d6832fcdab30a2b5e7d3824fe0ca73889a0f8bcfd02e9bb23ffbb0a6a1a0e`
- H.264, 1280×720, 30 fps, exactly 10.000000 s, 300 readable frames, 8,066,018 bytes. Full FFmpeg `-xerror` decode passed with an empty `decode.log`.

The first/5th/9th-second stills all show in-level forest gameplay, player, HUD, distinct camera positions and progression (orb HUD 3 → 5 → 5); none is a lock screen, menu, setup screen or desktop. The capture began only after both the named game window and `Audio level Game/XmlFiles/levels/forest_exploration_1.xml authored_ambience=2` entry log appeared. The recorder captured the game window by ID (`0x4a0000a`), not the desktop. The direct `--play` launch bypassed the menu. These gates and sampled frames support active-only footage; this was not a manual visual inspection of every individual frame.

## Provenance and method

No competing Death Leak game, FFmpeg, or build process was present at the pre-build/pre-capture checks. Forced `make -C port -B -j2` passed in `deathleak-linux-build:bookworm`; the capture binary SHA-256 is `8fd70a7f8d0594f7e748556494f267b4a6c8575f378686e0596ea7a7d6bc1347`. The copied, uniquely pathed `record.sh` launched `--play Game/XmlFiles/levels/forest_exploration_1.xml --script B:300,F:99700 --frames 100000 --fixed-dt 0.0166667`, gated on the game-window and level-entry log, then recorded 10 seconds with FFmpeg x11grab `-window_id`.

The game ran in `deathleak-video-capture:bookworm` against physical Xorg `:0`, with `/dev/dri` and read-only LightDM Xauthority. Its log reports SDL `video_driver=x11 name=opengl flags=14 accelerated=1 software=0 vsync=1 mode=strict-accelerated`, OpenGL `nouveau / NVE7`, GPU Sepia.fx, 71 original sprites, zero missing/unsupported assets and two authored ambient voices. FFmpeg reported an x11grab shared-memory issue, fell back to non-shared-memory capture, and completed successfully; the full decode and 300-frame count verify the resulting file. Dummy audio was used, so this is visual-review footage, not an audio deliverable. No game/FFmpeg process remained after capture.

The ignored evidence directory contains `head.txt`, `binary.sha256`, `build.log`, `record.sh`, `game.log`, `window.txt`, `ffmpeg.log`, `ffprobe.txt`, `final-ffprobe.txt`, `decode.log`, and 1/5/9-second PNG stills. No source code or shared status file was edited by this lane. Existing unrelated worktree changes were left untouched; this unique report is the only non-ignored output.

## Limits and handoff

The footage verifies the current corrected renderer in active gameplay, not matched original-Windows visual parity or user acceptance. No matched Windows camera/state/time frame was available. The private Discord destination could not be verified in this child context, so **the MP4 was not sent to Discord**. The coordinator should use a verified private target and an attachment-capable message tool, then retain its delivery receipt. The local path alone is not proof of delivery.
