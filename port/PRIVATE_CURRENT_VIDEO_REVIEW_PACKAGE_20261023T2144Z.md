# Private current video review package — 2026-10-23 21:44Z

## Delivery decision

**Use the newest saved physically accelerated active-core-gameplay MP4 for Jerome's qualitative visual review:** `port/build/knife-trajectory-gpu-20261023/forest-knife-trajectory-10s.mp4` (SHA-256 `a44f5d33395c865da0ef66970cdba6320a409bd7af0627624898667eeda26e87`). It is **not** an exact recording of current HEAD `29984eb21b489e59c944e021b14204f8d10d3446`, because the MP4 and binary were produced before the subsequent stored-Running-rate-order commit. This freshness qualification is the sole material blocker to calling it a *current-HEAD* video; it does not block sending it expressly as a recent Linux visual-review clip. No matched-original visual parity or Jerome acceptance is claimed.

## Artifact, content, and provenance checks

- `ffprobe.txt`: H.264/yuv420p, 1280×720, 30 fps, 300 decoded frames, exactly 10.000000 s; MP4 is 5,101,415 bytes (4.865 MiB), below an 8 MiB attachment ceiling. Saved full-decode `decode.log` and transcode log are empty; `ffmpeg.log` has nonfatal x11grab shared-memory/rate-estimate warnings. The direct-window source MP4, 1/5/9 s stills, recorder script, window evidence and logs are alongside the attachment.
- `record.sh` launches `--play Game/XmlFiles/levels/forest_exploration_1.xml --script B:300,F:99700 --frames 100000 --fixed-dt 0.016666667`; it waits for the named game window and Forest-1 entry log, settles 0.5 s, and records the actual 1280×720 window for 10 s. `window.txt` confirms a viewable named window. This is scripted active gameplay, not a physically pressed-key demonstration.
- `game.log`: strict SDL X11/OpenGL `accelerated=1 software=0`, nouveau/NVE7, GPU Sepia before HUD, 71 original level sprites, zero missing/unsupported assets, Forest-1 ambience. The 1/5/9 s stills and later read-only reviews show uninterrupted Forest bridge-to-ground traversal, moving player/camera, shots and orb HUD count 2→5. No menu/setup/blank frame or missing-art presentation is evidenced; the clip does not reach Waterfall. This is perceptually usable at native size, although player silhouette is conspicuously dark under/near the tree around 9 s and the orb numeral is blocky. The dark overlap is the already owned authored-ID10/ID37 question, not a proved new defect; the bitmap-vs-Impact numeral is a known source-face gap.
- Saved executable SHA-256 `bfd76e7b246da84156953fabed7e8e80b53aa539eac2fcfd37cffdc54a57e190`. Its mtime 14:30:58 PDT and MP4 mtime 14:31:32 PDT precede knife-pitch commit `a4fc2e3` at 14:31:52 and Running-rate commit `29984eb` at 14:44:38. The corrected-knife source was evidently in progress before commit, but no frozen-source manifest or build-to-commit attestation is present in this packet. Therefore **do not assert that the binary was built from `a4fc2e3` or `29984eb`**; describe it as the latest saved pre-Running-rate physical-GPU capture with knife-trajectory changes, supported by the capture packet and subsequent source review. Current tracked dirty renderer and other shared files remain unqualified relative to that binary.

## Sending gate and concise review request

There is no clip-content, codec, duration, size, or physical-acceleration blocker to private delivery. The private sender must attach the exact MP4 above, verify the attachment is present, and avoid labeling it exact current HEAD or original-Windows parity. If Jerome specifically requires a literal post-`29984eb` current-head video, a new separately owned build/capture is required; this read-only lane did not make one.

Suggested private caption: **“Here’s a 10-second Linux Forest-1 gameplay clip from the latest saved physical-GPU capture (5.1 MB). Please review the bridge-to-ground motion, knife/projectile readability, HUD, and especially whether the player remains readable by the dark tree near 9 s. It predates the latest Running-animation timing commit, so this is visual feedback on the saved clip, not final current-HEAD or Windows-parity sign-off.”**

## Scope

Read-only audit of existing MP4 packet, reports, Git log/status and file metadata. Only this unique report was written. No source/test/shared-status edits, build, Docker, game, Xorg/GPU use, footage capture, index/commit/push, or message. Existing dirty files preserved.
