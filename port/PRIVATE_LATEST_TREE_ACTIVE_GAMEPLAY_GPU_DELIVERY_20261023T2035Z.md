# Private latest-tree active Forest gameplay GPU capture — 2026-10-23 20:35 UTC

## Verdict / deliverable

**Verified 10-second current-tree private visual-review clip:** `port/build/private-latest-tree-active-gameplay-20261023t2035z/forest-current-tree-10s.mp4` (SHA-256 `32a844ff7f3bca72ecdcd1157dd69926ee439c744a7fd79804b03f045a637a76`). It is continuous *active Forest-1 gameplay*, not menu/setup: the reused recorder waited for both the named game window and the Forest-1 entry log, settled 0.5 s, then captured the physical Xorg `:0` window. Sampled decoded stills at 1/5/9 seconds show moving player/camera and authored bridge-to-forest traversal, firing, orb HUD 2→5, and no menu or blank/missing art. It remains qualitative Linux review evidence, **not** exact original-Windows parity or Jerome acceptance; the player is low-contrast behind the authored foreground/trunk at 9 s, and orb numerals retain the known bitmap-vs-Impact gap. No external message/upload was made in this lane.

## Exact tree/build provenance

- Built from HEAD `6dfc65ba086b3009809f9b303d3eccf391818a34` (the original opaque-black active clear is already its sole commit change) plus the then-dirty working-tree source. This includes preexisting `port/game/ParticleSpray.h` and `port/linux/SDLRenderer.cpp`; no source edits were made here. The working-tree source and Game assets were copied to the isolated `snapshot/`, frozen and built read-only with `deathleak-linux-build:bookworm` using `make -e -C port TARGET=/out/deathleak-linux -j2`. A separate writable build-work mount accommodated the Makefile's generated `build/` directory. Build exit 0; only existing TinyXML fallthrough warnings. Frozen binary SHA-256 `95b67d671ab4a1aa62e8181c5c084080922b648b52cf8825a5ecb9a0c0739e7b`.
- Source hashes before/after, full snapshot hashes, HEAD before/after, tracked diff before/after, status before/after and zero-line recursive drift checks for Game assets and participating `linux/game/render/audio` sources are in the evidence directory. HEAD and tracked diff were stable, source manifests match, and the snapshot matches live assets/source after capture. Unrelated dirty/untracked files were preserved.
- Copied the proven 19:39 `record.sh` unchanged (SHA-256 `96dfe00c0511e2c10800bc5433bc61649a66b1914f3b2c3d6243a2598b88ba2c`); only its `/src` input now names the frozen snapshot. Launch used `deathleak-capture:post-foliage`, physical `/dev/dri`, host network/identity, LightDM Xauthority, Xorg socket and `DISPLAY=:0`. The script runs `--play Game/XmlFiles/levels/forest_exploration_1.xml --script B:300,F:99700 --frames 100000 --fixed-dt 0.016666667` with dummy audio.

## Media/GPU gates

- `game.log`: strict `video_driver=x11 name=opengl accelerated=1 software=0`, `vendor=nouveau renderer=NVE7`, GPU Sepia before HUD, 71 original Forest level sprites, 0 missing/unsupported assets and authored Forest audio-level entry.
- `window.txt`: 1280×720 named game window. `ffprobe.txt`: H.264/yuv420p, 1280×720, 30/1 fps, **300 decoded frames, 10.000000 seconds**, 4,975,896 bytes. Full `ffmpeg -xerror` decode exited 0 with empty `decode.log`; transcode exited 0. The direct-window MPEG-4 source and both FFmpeg logs are preserved. x11grab's shared-memory/rate-estimate warnings were nonfatal.
- Decoded still SHA-256: 1 s `28f53eac2892bd3873538b6e9399a5ea6eedaf033a10696d24a67a92ec9279b4`; 5 s `079584113e0aef869bca677ca63263f78c963d5c8dab7f057b16cb55dee76822`; 9 s `91b7a81e5c9c34e2d4b9a5a30a7778c75c6f8cd2d570aee24ec9f9f0bd877a96`. All evidence is ignored under `port/build/private-latest-tree-active-gameplay-20261023t2035z/`.

## Lane release

The build and capture containers exited; no Death Leak game or FFmpeg process remained. No shared `PORT_WORK_STATUS.md`, source, Git index, commit or push was changed. The black-clear patch was already committed in HEAD; there is no owned uncommitted source change to stage. **Exclusive GPU/build/capture lane released.**
