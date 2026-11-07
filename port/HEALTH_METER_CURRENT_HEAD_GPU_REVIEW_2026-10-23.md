# Current-HEAD health-meter physical-GPU review — 2026-10-23

## Scope and preflight

HEAD `ec5c509` includes the health-meter implementation. The shared checkout also had unrelated dirty LevelScene, main, integration/test, status and report files; none was edited or staged. Before build/capture, host `ps` showed physical Xorg `:0` but no Death Leak executable, FFmpeg/x11grab, compiler, or other game capture process. Docker had only unrelated mobile-games and OpenClaw sandbox containers. A second process check immediately before capture also showed no competing GPU owner.

A fresh current-tree executable was built in isolated ignored `port/build/health-meter-current-head-gpu-2026-10-23/` using `deathleak-linux-build:bookworm`, read-only `/src`, and `make -e -C port TARGET=/out/deathleak-linux -j2`. Build exited 0 with existing TinyXML warnings only. Executable SHA-256: `631f7a77c6886925036307bc719129f9875af6f2c26fe3597f29d47c7d066f96`. This binary includes the checkout's unrelated dirty source, so it is a **current working-tree** review with HEAD's committed HUD, not a clean-HEAD isolation claim.

## Physical-GPU capture

`record.sh` in the evidence directory launched that exact binary on physical Xorg `:0`, bound `/dev/dri` and LightDM Xauthority into `deathleak-capture:post-foliage`, and waited for both the named game window and authored forest-1 level-entry log before X11 window-ID capture. It used forest-1 `--script B:300,F:99700`, fixed step `0.0166667`, dummy audio. The game window was 1280×720. `game.log` reports SDL `video_driver=x11`, OpenGL accelerated=1/software=0/strict-accelerated, nouveau/NVE7 GPU renderer, GPU Sepia.fx, 71 original level sprites, zero missing or unsupported assets, and authored forest ambience=2. Thus this was hardware-accelerated Linux/Xorg gameplay, not a software-rendered fixture.

Artifact: `port/build/health-meter-current-head-gpu-2026-10-23/forest-1-active-5s.mp4` (6,044,862 bytes; SHA-256 `23916935d3f7434f53bfd9ac3aec00ce7119d6dbc23075acff6c7d4e963c8c23`). `ffprobe.txt` shows H.264 1280×720 at 30 fps, 150 readable frames, 5.000 seconds. An independent `ffmpeg -xerror` full decode exited 0 with empty `decode.log`. `still-2s.png` (SHA-256 `0d3743bf993da61a35637e4b6e95ba1cb6b9790c2f879c3dd9b3a2877c8f2d28`) is an active forest gameplay frame after level entry. FFmpeg noted an X11 shared-memory event error but continued without shared memory; recording and decode completed. The game/recorder exited, and the final process check found no remaining game or FFmpeg process; GPU lane released.

## HUD visual gate

The current still shows the upper **red health meter** at roughly x=39–85, y=42–51: about 50 logical pixels including caps. Its left and right ends are visible, and the short red fill remains legible. The **cyan focus meter below** remains roughly x=39–163 and is not shortened. The white orb icon and count (`3`) remain readable below. The cap is present at the shortened right end; the crop does not support finer claims about texture orientation or subpixel alignment.

For comparison, the saved pre-fix `port/build/current-tree-gpu-review-0556-2026-10-23/still-5s.png` shows the old red health meter extending to about x=224 (~185 pixels), while focus still ends near x=163. `current-hud-crop.png` and `prior-hud-crop.png` in this evidence directory show those HUD regions enlarged. The older frame is from a different camera/time and source state, so this is a HUD-geometry comparison only, not a registered full-frame visual-parity comparison.

The observed short length matches the source contract: original `UIGameHudScreen.cpp` target meter length is `3 × maxHealth`, `UIMeter.cpp` uses `(length - 0.8 × edgeWidth) × progress` for fill, and the port's `Hud.cpp` projects the outer length by 2/3 into the 1280-wide viewport. The implementation's 25-health settled target is 75 original UI units / 50 logical pixels. This live still verifies the intended gross length, cap placement, and in-scene readability, not every transient upgrade tick or health-damage state.

**Result:** bounded physical-GPU HUD visual gate passes for the current working tree. No Windows matched-frame parity or Jerome human acceptance is claimed.
