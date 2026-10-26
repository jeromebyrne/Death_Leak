# Post-HUD multiscene sealed-replay acceptance gate — 2026-10-26

Base: live `origin/feature/linux-renderer-foundation` at `3b3f5086f3e6cfbe2d6b0cf1c705ef0d776add3b`, in an isolated clean worktree. The divergent dirty primary checkout was not built or edited except append-only checkpoint entries.

## Scope and method

`port/tests/posthud_multiscene_replay.sh` runs the same fixed-60-Hz, ordinary scripted-input route twice for each case. The control captures its final ordinary frame; the repeat arm captures both its ordinary frame and an immediate complete sealed-command replay, after all scene postprocessing, gameplay film, HUD, modals/loading UI, and without an FPS overlay. It checks 32-bit 1280×720 BMP byte identity across all three arms (therefore 0/921,600 changed pixels), exact scene/transition identity, and equal gameplay endpoint, combat and transition logs (excluding elapsed wall time). Test film clock is synchronized at 100 ms/update in both arms; the default game clock is unchanged. This is software/offscreen functional evidence, not a physical display cadence measurement.

Cases: Forest combat input at frame 179 (11 shots fired); Forest→Waterfall loading-fade frame 415 after authored transition at 398; later Waterfall-top frame 500; Waterfall-bottom frame 780 after authored transition at 762. All four capture triplets were pixel-identical. The four distinct frame hashes and reproducible output logs are listed below.

## Side-effect boundary

A source audit found `PollQuit()` at the top of the main loop, before its no-fixed-update refresh-only branch. That polled input/events on each repeated display slot. The scoped fix moves it after the due/update decision: refresh-only slots now consume only the sealed renderer packet and presentation-surface check, then continue; the ordinary fixed update retains one event pump. The existing 100,000-repeat packet fixture now asserts zero event polls as well as zero input reads and texture loads, with a positive one-poll fixed-update control. Baseline-vs-repeat game endpoints include player/camera-dependent progression, health, focus, currency, shots, contacts, melee/deflection and transition history. The service has no gameplay, input, audio, capture, camera, film or shader-clock dependency; those updates sit below the fixed-update branch. This establishes no repeat invocation/mutation on the service path; it is not a separate instrumented real-audio-device event count.

## Physical mode blocker

Xorg `:0` is present but inaccessible to the unprivileged `openclaw` user: `DISPLAY=:0 XAUTHORITY=/home/openclaw/.Xauthority glxinfo -B` returns `Invalid MIT-MAGIC-COOKIE-1 key` and `Error: unable to open display :0`. The LightDM root authority file is permission denied and passwordless sudo is unavailable. Consequently no selected >60-Hz physical display, nouveau/NVE7 GPU, hardware-input, sustained FPS, or release acceptance is claimed. The offscreen OpenGL software renderer is only a functional pixel gate.

## Evidence

- Final isolated Docker production build: PASS (`deathleak-linux-build:bookworm`, `make -C port -j2`).
- `make presentation-command-packet-test refresh-slot-scheduler-test hud-packet-cpu-test`: PASS.
- Captures/logs: `port/build/posthud-multiscene-final/` in isolated worktree; reproducible with `port/tests/posthud_multiscene_replay.sh` (or `DEATHLEAK_VERIFY_ONLY=1` for retained outputs).
- `git diff --check`: PASS.

| Route/capture | SHA-256 of each of three byte-identical BMPs |
|---|---|
| Forest combat, frame 179 | a165e9570e413fcd8bd025c19ab504205ad4540e5c70fa8e4cb2ee08484e02cd |
| Waterfall fade, frame 415 | 69245599006376b51805c0e3d502d1a1e64e40ce6cedcd2c2c0e6b9067fe424b |
| Waterfall active, frame 500 | b72d5b780cf4fc22cec6ce6301a242513ac1a39cf6e7f1ea0b37d2056177f54a |
| Waterfall bottom, frame 780 | 214f94a64c5d1991080dc5b8458f689098822616d5cde36798168e764d5f2aee |

Next release gate is authenticated physical Xorg authority and actual selected >60-Hz mode with on-device slot/update/event counters, full route captures, and sustained frame-time/thermal evidence. Atlas/bind microgates are outside this acceptance slice.
