# Forest-1 current-cap delivery capture — 2026-10-23

## Result

**Ready for coordinator's private gameplay review delivery.** A new 10-second active forest-1 gameplay clip was captured on physical Xorg `:0` using strict accelerated OpenGL (`nouveau/NVE7`, `accelerated=1 software=0`) and GPU `Sepia.fx`. H.264 1280×720 at 30 fps decodes cleanly for **300 frames**, duration 10.000000 s. This is a qualitative current-tree review clip, not original-Windows pixel parity or human acceptance. No upload/send was performed in this lane.

## Frozen provenance and capture gate

- HEAD `22950f87a0c37207f234b859506759ec0c3f3ca3`; source tree had pre-existing dirty edits. No source edit/build/commit/push occurred. Copied `port/build/deathleak-linux` to ignored `port/build/forest-current-cap-delivery-capture-2026-10-23/deathleak-linux`; both SHA-256 `70739562d96395f4731674b0ad64a356e6d94f20d18d306bed9ff40269ebbacb`. This binary includes the cap at HEAD and the pre-existing dirty renderer state; it is not asserted to be a clean-HEAD build.
- Reused the proven `post-render-delta-cap-gpu-review` `record.sh` in the ignored output directory (copy SHA-256 `96dfe00c0511e2c10800bc5433bc61649a66b1914f3b2c3d6243a2598b88ba2c`). Capture image `deathleak-capture:post-foliage`, image ID `sha256:6b38f2bd1f9cd1543956d70dd857ced86ff50796982a2a9bb32711fce87668b7`. Docker had read-only repository mount, writable ignored evidence output, `/dev/dri`, physical Xorg socket and LightDM `:0` Xauthority. No competing game, FFmpeg or GPU capture was observed before starting; all capture processes had exited afterward.
- Invocation: `--play Game/XmlFiles/levels/forest_exploration_1.xml --script B:300,F:99700 --frames 100000 --fixed-dt 0.016666667`. Recorder waited for the named game window **and** logged forest level ambience entry, then settled 0.5 s before `x11grab` direct-window recording. Thus the 10-second recording began after in-level gameplay had started, not at launch/menu. Game log reports 71 original sprites, zero missing/unsupported assets, strict accelerated SDL `x11/opengl`, and GPU sepia postprocess.

## Exact deliverable and checks

- **Review MP4:** `port/build/forest-current-cap-delivery-capture-2026-10-23/forest-current-tree-10s.mp4` — 4,682,487 bytes; SHA-256 `b202f5e2fdfd165ce812422a13eb35657b595f948515ebcce9edaae27b2bff89`.
- Original direct-window recording: `port/build/forest-current-cap-delivery-capture-2026-10-23/forest-current-tree-10s-source.mp4` — SHA-256 `486ffa8679750b320a7c145864a2187147084e8a81a9a931c79353e1ddf34c02`.
- `ffprobe.txt`: H.264, 1280×720, `30/1`, `nb_read_frames=300`, `duration=10.000000`. Full `ffmpeg -xerror` decode passed with empty `decode.log` (0 bytes). `game.log`, `window.txt`, `ffmpeg.log`, `transcode.log`, recorder and stills are retained alongside the clip.
- Sample stills, 1/5/9 seconds respectively: `still-1s.png` SHA-256 `7a96ea3819d9f4453a2582988d0d19e54aad8f6178d180432aa9efcb1df015f5`; `still-5s.png` `1df558346f5e1975fd27f2f398959afd0a3569213175eb4ada0e1b3b6241ca7f`; `still-9s.png` `1c7b243cd2b1a2e5f9df26985948afb0f9b6774c2f3cf34dd90a5417a0a6dcbd`.

## Visual review

At 1/5/9 seconds the character progresses from the bridge through large trees with pickups/HUD visible (orb display advances from 2 to 5), so the sampled clip is active gameplay, not static splash or desktop. The layered sepia forest, trunks, branches, bridge, orbs and moving camera remain coherent; no blank screen, missing level art, obvious HUD loss or gross new composition break is visible in these samples.

Remaining review questions are **qualitative**: dense near-black lower foreground covers a substantial part of the ground; ID37's authored contribution was already causally demonstrated in `ID37_CURRENT_CAP_GPU_DISCRIMINATOR_2026-10-23.md`, so this clip does not justify removing/recoloring it. The right side intermittently has a broad pale shaft/field with a fairly straight-looking transition; prior source attribution points toward authored ID15, but this wall-clock clip is not a matched original-state discriminator. At 9 s the player is hard to see against/behind the very dark central trunk and lower foreground. These are visibility/composition topics for Jerome's review, **not proven Linux regressions**. No ID37 or middle-band edge audit was repeated. Exact Windows parity needs matched lossless original and Linux frames at the same player/camera/effect state.
