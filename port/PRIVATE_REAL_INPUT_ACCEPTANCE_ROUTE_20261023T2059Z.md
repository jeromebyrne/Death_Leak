# Private real-input acceptance route — 2026-10-23 20:59Z

## Next gate and evidence boundary

After the delivered active Forest knife/FPS clip, the next distinct **physical-input** acceptance gate is a locally attached keyboard's held **D/Right** moving the player in the focused, no-script Forest-1 game on the physical `:0` display. This is the smallest actuator proof before controller, right-stick, mouse, audio-listener, or broader human-play claims. `SDLRenderer::PollQuit` pumps SDL events and `KeyDown(Right)` reads `SDL_SCANCODE_D/RIGHT` (also gamepad D-pad/left X) (`port/linux/SDLRenderer.cpp:745-766`); no-script gameplay consumes that state for `PlayerMotion::Update` (`port/linux/main.cpp:753-754,813-814`), and the terminal `Simulation ended ... player=(x,y) ... orbs=...` is the measurable endpoint (`main.cpp:1497-1506`). The existing strict-Xorg scripted route is *not* real-input proof.

`CHECKPOINT_STATUS.md:776-778` already records a 420-frame idle Forest baseline x≈-43.01/one orb and a scripted `R:420` route into Waterfall/seven orbs. A focused XTest-held Right registered in Xorg but produced no SDL scancode/event and matched idle; that synthetic result is **inconclusive for a real keyboard**, not evidence of physical-key failure. `/dev/input/js0` was the Apple SMC sensor, not a gamepad; no real controller was enumerated (`CHECKPOINT_STATUS.md:712,776-777`). The earlier `port/PRIVATE_POST_CLIP_REAL_INPUT_GATE_20261023T2032Z.md` nominated this same gate but did not execute it. The current ignored binary exists, yet its timestamp alone cannot bind it to HEAD `c992427` or concurrently dirty code; no acceptance is claimed here.

## Exclusive physical-display owner procedure (not run here)

Prerequisites: the audio/vocal QA owner has released its files/build; one GPU/Xorg owner has an unlocked physical `:0`, a **person and locally attached keyboard**, a focused game window, Docker image `deathleak-linux-build:bookworm`, readable LightDM `:0` authority, `/dev/dri`, and the existing strict Xorg/NVE7 route working. Freeze the intended source revision and record `git rev-parse HEAD`, dirty-file hashes (or a clean-tree receipt), and `sha256sum port/build/deathleak-linux`; build that exact tree under the exclusive owner (forced/clean if header changes could leave incremental `make` stale). Run from repository root, with no `--script`, `--simulate-only`, XTest, virtual-input injection, or concurrent GPU worker. In `idle`, touch no controls; in `key`, physically hold D for ~3 seconds once Forest is visibly active, then release. Keep the window focused and observe actual rightward on-screen motion. Use a fresh process for each run:

```sh
gate_run=idle # Run once; repeat the entire command with gate_run=key after preparing the operator.
timeout --signal=TERM --kill-after=10s 50s docker run --rm --network host \
    --hostname jerome-Linux-MacBookPro --device /dev/dri \
    --mount "type=bind,src=$PWD,dst=/src" \
    --mount type=bind,src=/tmp/.X11-unix,dst=/tmp/.X11-unix \
    --mount type=bind,src=/var/run/lightdm/root/:0,dst=/tmp/deathleak.Xauthority,readonly \
    -e DISPLAY=:0 -e XAUTHORITY=/tmp/deathleak.Xauthority \
    -e SDL_VIDEODRIVER=x11 -e SDL_RENDER_DRIVER=opengl -e SDL_AUDIODRIVER=dummy \
    -w /src deathleak-linux-build:bookworm port/build/deathleak-linux \
    --play Game/XmlFiles/levels/forest_exploration_1.xml \
    --frames 420 --fixed-dt 0.0166667 \
    >"port/build/physical-real-input-gate-$gate_run.log" 2>&1
printf '%s exit=%s\n' "$gate_run" "$?"
```

Run the command once with `gate_run=idle`, then rerun it with `gate_run=key` after the operator is ready; do not auto-loop the two processes. Inspect each full log for `video_driver=x11 name=opengl ... accelerated=1 software=0 ... mode=strict-accelerated`, `OpenGL device vendor=nouveau renderer=NVE7`, 420-frame completion and the final `Simulation ended` level/position/orbs. Preserve both logs and binary/source identity in a unique acceptance receipt. Compare fresh idle versus key: pass this **D/Right path only** if the key run visibly moves right and reaches at least +100 world x relative to idle, or enters the rightward Waterfall scene with more earned orbs. If confirmed physical hold and window focus yield no visible response and idle-equivalent final state, fail keyboard delivery; if focus, actuation, source/binary identity, strict GPU, process exit, or frame completion is uncertain, mark inconclusive. A passing D key does not accept controller/right-stick, all gameplay controls, or parity with Windows (where keyboard gameplay was disabled).

## Status

Read-only audit plus this uniquely named report only. No build, GPU/Xorg/display access, capture, source/test/shared-status edit, commit, or input test was performed; ongoing vocal QA and unrelated dirty files were untouched. Physical actuation remains the blocking prerequisite and the above route is a handoff, **not an acceptance result**.
