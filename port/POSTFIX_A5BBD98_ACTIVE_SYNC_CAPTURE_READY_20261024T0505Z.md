# HEAD a5bbd98 active Forest→Waterfall synchronized capture — CPU-only preflight

**Status: prepared, not run.** The performance owner occupies physical Xorg `:0`/NVE7 and the build lane. This worker did not acquire either lane, build, start a game, run FFmpeg, or use a GPU; lane released (never acquired). Existing dirty `CHECKPOINT_STATUS.md`, `port/linux/SDLRenderer.cpp`, `port/tests/integration.sh`, and untracked work were left untouched. No production/shared file, commit, or push was made.

## Ready invocation, after explicit owner release

From `/srv/openclaw/workspace/death_leak_analysis/source`:

```sh
DEATHLEAK_GPU_RELEASED=yes ./port/postfix_a5bbd98_active_capture_preflight_20261024T0505Z.sh
```

The release variable is an operator assertion, not an automated GPU-ownership detector. **Do not set it until the performance owner has released physical `:0`/NVE7 and build.** Script refuses a changed HEAD; expected full commit `a5bbd981dccb371fe4b2b266a187164fa86dffaa` (`Preserve unbounded waterfall wobble V phase`). It freezes current-tree source hashes and `git status` in unique ignored `port/build/postfix-a5bbd98-active-<UTC>-<PID>/`. It builds with `deathleak-linux-build:bookworm` from read-only `/src`, two read-only diagnostic source overlays, and isolated writable `/out` (`make -C port TARGET=/out/deathleak-linux all`). Nothing overlays or edits the shared working tree. Current-tree dirty renderer content, if still present at execution, is part of the binary provenance and not a clean-HEAD claim.

The serial capture uses the earlier proven `deathleak-capture:post-foliage` route: host Xorg `DISPLAY=:0`, LightDM Xauthority, `/dev/dri`, `SDL_VIDEODRIVER=x11`, OpenGL strict accelerated NVE7, scripted `--play Game/XmlFiles/levels/forest_exploration_1.xml --script B:1800 --frames 100000 --fixed-dt 0.016666667`, dummy audio, and FFmpeg `x11grab -window_id ... -framerate 30 -t 45 -vf scale=1280:720 -an -c:v mpeg4 -q:v 4 -pix_fmt yuv420p`. Recorder starts only after Forest entry/window and stops five seconds after the first active Waterfall-bottom present; this encompasses the active top paired-raft interval near video 20 s in the prior 26 s route. A same-run EndFrame CSV carries frame, steady/realtime nanoseconds, level/loading/gameplay flags and camera/player XY; `ffmpeg -debug_ts` source packet timestamps are aligned to latest completed presents in `alignment.json`. The script retains `record.sh`, trace overlays/hashes, build/game/FFmpeg logs, 1/9/11/14/18/20/22/24-second stills, ffprobe, full-decode log, MP4 hash, and source-state snapshots.

**Post-fix ID13/14 gate:** isolated `LevelScene.cpp` overlay logs each viewport-submitted Waterfall-top pixel-wobble ID 13 or 14 with *unbounded* `draw.uvShiftY`, `draw.pixelWobble.time`, and draw camera XY to `id13_14_uv.csv`. The script fails unless both IDs submit V > 1 after first integer crossing, all Forest/top/bottom active presents exist, at least 30 source packets follow active bottom, full MP4 decode succeeds, and game log proves accelerated X11/OpenGL `nouveau/NVE7`. This observes submitted shader inputs, **not** a measured same-present RGB delta or matched Windows D3D parity; those require separate controlled pixel evidence. Prior synchronized route: `port/SYNCHRONIZED_ACTIVE_WATERFALL_VISUAL_PACKET_20261024T0250Z.md` (pre-fix HEAD `0a69203b`, not a post-fix capture). Prior lane gate: `port/PRIVATE_WATCHDOG_TOP_WOBBLE_POSTFIX_NVE7_LANE_BLOCKED_20261024T044231Z.md`.

**Verification now:** `bash -n` on outer script and `sh -n` on embedded recorder passed. No build, source overlay generation, or capture was executed. Script/report are the only new scoped files. GPU/build/capture lane remains with the performance owner; this preflight lane is released.
