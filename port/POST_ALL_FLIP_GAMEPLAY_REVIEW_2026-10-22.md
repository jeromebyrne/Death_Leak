# Post-all-flip active forest-1 gameplay review — 2026-10-22

## Result and artifact

A **new** 10-second forest-1 active-gameplay video was captured from HEAD `c1725ee18315ab8a6192f8be8b227e58386206d7` after the all-four-flip fix:

- MP4: `port/build/post-all-flip-gameplay-2026-10-22/forest-1-post-all-flip-active-10s.mp4`
- SHA-256: `3468221e877ef0b38a1299c3733bf642b41ecd2eda68a39e51fc6c0a25bbc5f6`
- H.264, 1280×720, 30 fps, **10.000000 s**, **300 readable frames**, 8,081,284 bytes. Full `ffmpeg -xerror` decode passed with an empty `decode.log`.

The 1-, 5-, and 9-second stills show live forest-1, player and HUD, with changing camera/player positions and orb HUD 3→5→5. At 1 s the player is on the bridge near the large tree; at 5 and 9 s the view is farther right, with the player in different poses/positions. These are gameplay frames, not a menu, desktop or setup screen. Sampling does not constitute frame-by-frame manual inspection.

## Capture provenance

A forced Docker build of the current worktree (`make -C port -B -j2`) passed before capture; binary hash and build log are in the ignored evidence directory. The established project `record.sh` recipe was copied to a unique path with only output path/name substitutions. It launched `--play Game/XmlFiles/levels/forest_exploration_1.xml --script B:300,F:99700 --frames 100000 --fixed-dt 0.0166667`, waited for both the game window and the named forest-1 level-entry audio log, then captured the **window ID** for 10 seconds with FFmpeg x11grab. Thus recording began after level load. Physical Xorg `:0` used read-only LightDM Xauthority and `/dev/dri`; the game log reports `video_driver=x11 name=opengl ... accelerated=1 software=0 ... mode=strict-accelerated`, OpenGL `nouveau / NVE7`, GPU Sepia.fx, 71 original sprites, zero missing/unsupported assets, and two authored ambience voices. Dummy audio means this is a visual-review clip, not an audio deliverable. FFmpeg fell back from shared-memory x11grab after an X error but completed; the independent full decode and frame count passed. No competing Death Leak game/encoder/build process was present at pre-capture inspection, and the capture container exited normally.

Evidence: `port/build/post-all-flip-gameplay-2026-10-22/{head.txt,binary.sha256,build.log,record.sh,game.log,window.txt,ffmpeg.log,ffprobe.txt,final-ffprobe.txt,decode.log,still-1s.png,still-5s.png,still-9s.png}`.

## Bounded visual comparison

The available Steam store screenshot `../store_media/screenshots/14.jpg` is a credible **forest-1 location/composition** reference near the bridge and large tree. The 1-second current still depicts the same recognizable bridge, broad tree/branch silhouette, hanging canopy, sepia forest, orbs, player and HUD. It is **not** a matched original-renderer frame: player/camera location, gameplay progression, HUD count, viewport presentation and effect time differ; the store screenshot is letterboxed. The original also visibly contains a bright ground-level haze in its frame, but this alone does not isolate the responsible layer or prove a current regression. The 5- and 9-second current stills are farther right and have no identified matching Windows camera/state frame. Existing controlled Linux ID122/foliage captures in `ALL_FLIP_ROTATION_FIX_2026-10-22.md` discriminate changed geometry, not original-Windows pixel parity. Therefore the clip supports active composite review and no obvious blank/menu failure; **it does not establish visual acceptance or exact Windows parity**. No concrete new source defect was proven by this bounded review, so no source was changed.

## Delivery and ownership

No verified private Jerome DM recipient or attachment-capable send tool is exposed in this child session; `CORRECTED_GAMEPLAY_PRIVATE_DELIVERY_ROUTE_2026-10-22.md` independently records the same blocker, and current tool discovery has no messaging/attachment sender. The MP4 was **not sent** or posted publicly; there is no delivery receipt. The coordinator should deliver this exact MP4 via a confirmed private attachment route and retain the message/attachment receipt, or report the route blocker to Jerome. GPU/game/encoder ownership is released after the finished capture; no running capture container was retained. Shared `CHECKPOINT_STATUS.md` and `PORT_WORK_STATUS.md`, source files, and unrelated worktree changes were left untouched.
