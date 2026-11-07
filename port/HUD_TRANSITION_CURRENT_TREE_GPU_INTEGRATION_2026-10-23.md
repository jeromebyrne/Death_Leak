# HUD transition current-tree integration — 2026-10-23

## Result

The source-backed HUD transition patch is integrated in the shared current tree, committed as `9ba65c3` (`Port HUD meter progress transitions and health backing`), pushed to `origin/feature/linux-renderer-foundation`, and passed focused CPU and Docker compile gates. Physical Xorg/NVE7 Forest-1 active gameplay capture passed. This is **not** a matched original-Windows transient comparison, pixel-parity claim, or Jerome acceptance. The 5-second scripted route holds ordinary gameplay and does not intentionally trigger a health/focus transition; transition behavior is CPU-spy/source-backed only.

## Owned changes and source review

The only software paths taken from the released CPU owner are `port/game/Hud.h`, `port/game/Hud.cpp`, `port/linux/main.cpp`, `port/tests/health_meter_length_cpu_spy_test.cpp`, and new `port/tests/hud_transition_cpu_spy_test.cpp`. `Hud` now updates independent health/focus progress state before registering new targets, interpolates over 0.25 s, snaps focus decreases, retains health's white previous-fill backing and its 0.005/update decay. `main.cpp` passes `shaderFrame.totalTime - dt`, matching original pre-increment process-total sampling. The existing health-length/cap behavior and focus/orb geometry remain covered by the spy. Original contract checked against `Game/src/UI/UIMeter.cpp:159-220`, `Game/src/UI/UIScreen.cpp:131-136`, `Game/src/UI/UIGameHudScreen.cpp:91-174`, and `Game/src/program.cpp:148-175`. Original Windows runtime transient pixels were not available/matched.

## Verification

- `g++ -std=c++17 -O0 port/tests/hud_transition_cpu_spy_test.cpp port/game/Hud.cpp -o /tmp/death-leak-hud-transition-cpu && /tmp/death-leak-hud-transition-cpu` — PASS.
- `g++ -std=c++17 -O0 port/tests/health_meter_length_cpu_spy_test.cpp port/game/Hud.cpp -o /tmp/death-leak-health-length-cpu && /tmp/death-leak-health-length-cpu` — PASS.
- `git diff --check --` the four owned tracked paths — PASS.
- Docker `deathleak-linux-build:bookworm`, read-only source mount, `make -e -C port TARGET=/out/deathleak-linux -j2` — PASS with existing TinyXML warnings. Isolated ignored output `port/build/hud-transition-current-tree-gpu-2026-10-23/deathleak-linux`; SHA-256 `ba78aa9a3b30848b869aca9ba17968f10f262e6dbf54ccccebfdfad1974f0777`. This is a **current working-tree** build that includes other dirty source, not a clean-HUD-only binary.
- Physical Xorg `:0` recorder used `deathleak-capture:post-foliage`, host network/hostname and LightDM Xauthority, `/dev/dri`, read-only source, and `record.sh` in the evidence directory. It waited for named window and Forest-1 authored-ambience entry before x11grab, so recorded gameplay has no menu/setup footage. `game.log`: x11/OpenGL strict accelerated, nouveau/NVE7, GPU Sepia.fx, 71 original sprites and zero missing/unsupported assets. `ffprobe.txt`: H.264 1280x720 30 fps, 150 frames, 5.000 s. Full `ffmpeg -xerror` decode PASS (`decode.log` empty). A first Xorg launch lacked host identity and failed authorization before capture; corrected launch exited 0. No game/FFmpeg process remained afterward.

## Artifact and limits

Active gameplay clip: `port/build/hud-transition-current-tree-gpu-2026-10-23/forest-1-active-5s.mp4` (6,225,090 bytes; SHA-256 `825308683c80ce77eef0bfd2e9ba10c7787b533ccb3bf808ecb081053185c2fd`). `still-2s.png`, logs, window metadata, `ffprobe.txt`, and recorder are beside it. No original-Windows frame with identical prior meter state, target tick, modified-time sequence, caps, and viewport was captured. The existing Windows source supports the state logic, but temporal pixel parity and visual severity remain open. Jerome acceptance remains open.

## Ownership

The commit contains exactly the five HUD software/test paths listed above; this handoff report remains untracked and was not part of the software commit.

HUD software ownership and exclusive build/physical-GPU lane released. `port/linux/SDLRenderer.cpp`, `port/tests/integration.sh`, coordinator/status files, unrelated reports, and the ID59 read-only source-gate report were not edited or staged. No integration.sh wiring was attempted because it remains dirty/owned elsewhere.
