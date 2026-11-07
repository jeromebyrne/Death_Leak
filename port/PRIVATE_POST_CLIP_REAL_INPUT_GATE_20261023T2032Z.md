# Private post-clip real-input gate — 2026-10-23 20:32Z

**Highest-priority unverified real-device behavior:** a focused physical keyboard's held **D/Right** key must reach SDL's gameplay `Right` state and move the Forest-1 player. This is the first actuator gate before interpreting controller/right-stick or broader human-play results. `SDLRenderer::PollQuit` pumps SDL events and `KeyDown(Right)` reads `SDL_SCANCODE_D/RIGHT` (`port/linux/SDLRenderer.cpp:745-766`); `main` uses that live state for movement when `--script` is absent (`port/linux/main.cpp:753-754,813-814`). The terminal `Simulation ended ... player=(x,y) ... orbs=...` line is a measurable observable (`main.cpp:1497-1506`).

Prior evidence: `CHECKPOINT_STATUS.md`'s 2026-10-22 01:07 PDT physical-Xorg diagnostic found idle Forest x≈-43.01/one orb over 420 frames and scripted `R:420` progressed to waterfall/seven orbs. Its XTest key press registered in Xorg but delivered **no SDL scancode/event**; that synthetic attempt cannot pass or fail a physical keyboard. `port/INPUT_AUDIO_GATE_AUDIT_2026-10-22.md` records no enumerated real game controller. Later physical-Xorg captures still use scripts. The current right-stick fire path is source-wired (`main.cpp:838-866`; `port/RIGHT_STICK_AIM_FIRE_IMPLEMENTATION_2026-10-22.md`) but also lacks live-device verification. No new input runtime test was done here.

**Exclusive GPU/input owner procedure (not executed):** With a locally attached keyboard and a person at the unlocked physical `:0` display, freeze/build the intended binary under that owner's normal procedure, then run this *twice* from repository root, changing `gate_run` from `idle` to `key`. Ensure no other worker owns Xorg/GPU and the game window has focus. Do not add `--script`, `--simulate-only`, XTest or virtual key injection. In `idle`, leave controls untouched; in `key`, after Forest appears, physically hold **D** for about three seconds, release, and let the process finish. Observe actual on-screen movement as well as the terminal result. The command uses SDL dummy audio to keep this an input gate:

```sh
gate_run=idle # repeat as gate_run=key
mkdir -p port/build/physical-real-input-gate
timeout --signal=TERM --kill-after=10s 50s docker run --rm --network host \
  --device /dev/dri -v "$PWD:/src" -v /tmp/.X11-unix:/tmp/.X11-unix \
  -v /var/run/lightdm/root/:0:/tmp/deathleak.Xauthority:ro \
  -e DISPLAY=:0 -e XAUTHORITY=/tmp/deathleak.Xauthority \
  -e SDL_VIDEODRIVER=x11 -e SDL_RENDER_DRIVER=opengl -e SDL_AUDIODRIVER=dummy \
  -w /src deathleak-linux-build:bookworm port/build/deathleak-linux \
  --play Game/XmlFiles/levels/forest_exploration_1.xml --frames 420 --fixed-dt 0.0166667 \
  >"port/build/physical-real-input-gate/$gate_run.log" 2>&1
```

**Pass:** both runs exit 0 and log strict `video_driver=x11 name=opengl ... accelerated=1 software=0` plus `OpenGL device vendor=nouveau renderer=NVE7`; the idle summary stays near its start (historically x≈-43/one orb), while the physical-key run visibly moves right and ends with materially greater `player` x (at least +100 world units relative to that fresh idle run), or traverses into the rightward Waterfall scene with increased earned orbs. If the key run has no visible response and its terminal position/orbs match idle despite confirmed window focus and held physical key, **fail** real keyboard delivery; if focus, device, strict GPU, frame completion or key actuation is uncertain, mark **inconclusive**, not a port failure. This accepts only the D/Right path, not jump, controller, aim/fire, mouse or input parity.

Read-only source/evidence gate: no build, GPU/Xorg, tracked-code edit or overlap with the audio source-parity worker. Unrelated worktree changes preserved.
