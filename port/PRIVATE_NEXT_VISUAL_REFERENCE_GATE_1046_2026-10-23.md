# Next visual reference gate — read-only handoff (2026-10-23 10:46 PDT)

## Decision

There is **no immediately executable matched-original-Windows frame route on this host**. The current checkout has no located original game EXE or synchronized Windows image/state packet; a bounded current probe found no `.exe`, `.rdc`, or `.wpix` under `/srv/openclaw/workspace/death_leak_analysis`, and no `wine`, `wine64`, `steam`, `msbuild`, or `dotnet` command. `WINDOWS_FOREST_RUNTIME_AVAILABILITY_2026-10-23.md` and `WINDOWS_MATCHED_FRAME_NEXT_ACTION_1647_2026-10-23.md` identify the same missing runtime dependency. The existing store JPEG and Linux MP4 have no atomic original-render state and cannot substitute. The conditional Windows route is: obtain an authorized D3D10-capable Windows host plus identified shipped app-995990 EXE (or separately labeled original-source EXE); hash EXE/assets; collect one lossless post-UI/pre-Present Forest-1 frame and same-ordinal state/inputs with the `FOREST1_MATCHED_FRAME_CAPTURE_PACKET_2026-10-23.md` contract. That is **not** an executable local command today.

The most immediate *later execution* is therefore the single-binary held-pull shake A/B after the exclusive burst GPU/build owner explicitly releases its lane. This is a handoff checklist, not a second audit or a captured result. The current `port/linux/main.cpp:697` unconditionally requests held sub-threshold shake; `:1231-1237` samples X-then-Y signs and passes the returned view into the world draw. CLI parsing at `:95-129` does **not** yet recognize `--capture-suppress-held-pull-shake`, so the OFF command below is **not runnable on the present binary**. `:1389-1392` saves `--frames N` as rendered loop frame `N-1` before EndFrame. The current source permits `--fresh-save-intro`, `--play`, `--script`, `--fixed-dt`, `--frames`, `--capture`; `--simulate-only` skips the visual sample. Forest-1 is follow-X true/follow-Y false; `CameraShake.h` snapshots view before changing stored camera position. No physical run or build was attempted here.

## Exact execution checklist for the released exclusive owner

1. After the burst owner releases shared build/Xorg/GPU, add only a **private capture-only** `--capture-suppress-held-pull-shake` parser flag in `main.cpp`, valid with fresh-save Forest-1 play and `--capture`, and guard only the `cameraShake.Start(...)` at line 697. Preserve zoom, timing, non-camera state, RNG, `Update`, landing shake, and HUD. Log ordinal, phase, held clock, accepted/refused Start, remaining/intensity, signs, camera drawn/stored XY, follow axes, zoom, player XY, and RNG/non-camera state. Do not ship the flag. Build once, freeze/hash the executable and asset/source manifest; both arms must use that exact binary, settings, seed, viewport and physical renderer.
2. For each `N` in `1 2 3 6 7 8 14 15 16 60 61 150 160 161 164 165 166 167 170 180 196`, run ON and OFF **serially**, each as a fresh process. The invocation inside the container is exactly:

   ```sh
   /out/deathleak-linux --fresh-save-intro --play Game/XmlFiles/levels/forest_exploration_1.xml --script I:220 --fixed-dt 0.0166667 --frames "$N" --capture "/out/$ARM-$N.bmp" $OFF_FLAG
   ```

   Here `ARM=on, OFF_FLAG=''` or `ARM=off, OFF_FLAG='--capture-suppress-held-pull-shake'`; set a distinct output/log name per process. Use the established physical route (not executed by this report):

   ```sh
   docker run --rm --network host --hostname jerome-Linux-MacBookPro --device /dev/dri \
     --mount "type=bind,src=$PWD,dst=/src,readonly" \
     --mount "type=bind,src=$PWD/port/build/PRIVATE_SHAKE_AB,dst=/out" \
     --mount type=bind,src=/tmp/.X11-unix,dst=/tmp/.X11-unix \
     --mount type=bind,src=/var/run/lightdm/root/:0,dst=/tmp/deathleak.Xauthority,readonly \
     -e DISPLAY=:0 -e XAUTHORITY=/tmp/deathleak.Xauthority \
     -e SDL_VIDEODRIVER=x11 -e SDL_RENDER_DRIVER=opengl -e SDL_AUDIODRIVER=dummy \
     -w /src deathleak-linux-build:bookworm \
     /out/deathleak-linux --fresh-save-intro --play Game/XmlFiles/levels/forest_exploration_1.xml \
     --script I:220 --fixed-dt 0.0166667 --frames "$N" --capture "/out/$ARM-$N.bmp" $OFF_FLAG
   ```

   Stage the frozen executable as `/out/deathleak-linux` and create the private writable output directory first. This Docker mount/env form matches `port/build/post-preceding-delta-current-head-gpu-review-2026-10-23/capture.sh`. Hash binary before/after; save each command/log/BMP. Require `video_driver=x11 name=opengl accelerated=1 software=0 mode=strict-accelerated`, nouveau/NVE7, GPU Sepia, 71 sprites and zero missing assets. Repeat on/off/on at one late held N to measure replay noise. Use a separately common `--script I:60,.:20,I:220` history for release-tail frames; do not mix that history with uninterrupted hold.
3. Compare **same-N** lossless BMPs only. N=1 is pre-visible negative control (view sampled before first stored jitter); N=2 is first possible displacement, potentially subpixel. N=166 is expected pull/zoom reset from the prior packet but must be re-confirmed in both traces; never compare N=165 to N=166 as a shake-only pair. Require matching player/phase/zoom/shader clock and non-camera RNG state, coherent horizontal signed displacement in at least two opaque static-world landmarks relative to drawn-view X (left branch around `(20,270,150,95)`, bridge around `(450,515,550,90)`), no systematic Y shift, and fixed HUD anchor. Animated foliage, particles, film and prompt must be masked. Differences after release can persist through follow lag; no whole-frame equality is required at N=180/196. A matched Linux causal pixel result is **not** original-Windows parity or Jerome acceptance.

**No-GPU fallback discriminator:** while the exclusive lane remains occupied, inspect only the current source and existing `port/tests/fresh_save_intro_camera_shake_cpu_test.cpp`/`port/FRESH_SAVE_INTRO_CAMERA_SHAKE_PRIMITIVE_2026-10-23.md`: their existing deterministic fixture proves active-start refusal, 0.1-s decay/expiry, X/Y follow gates and view-before-stored phase. Together with `main.cpp:697,1231-1237` and Forest XML follow flags this predicts first eligible world-view divergence no earlier than N=2, X only, fixed HUD; it cannot prove a rendered pixel delta. If a later CPU-only trace is allowed, compare ON/OFF at equal ordinals for the first differing drawn-view-X while asserting identical player/phase/zoom/non-camera state; reject causal attribution if those differ. Do not invoke `--simulate-only` as a visual surrogate.

## Scope release

This uniquely named private report is the only write. No source/test/status/checkpoint edit, build, execution, capture, GPU/Xorg use, commit or push occurred. **Read-only next-visual-reference ownership is explicitly released now.** The exclusive burst GPU/build worker retains its lane until its own explicit handoff; this report reserves none of it.
