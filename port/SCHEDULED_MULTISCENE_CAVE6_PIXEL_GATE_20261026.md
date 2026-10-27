# Scheduled production-present multiscene cave-6 gate — 2026-10-26

## Scope and reproduction

Live GitHub `feature/linux-renderer-foundation` was verified at `4bc3ec403cbcd913580231f84ffbd6b4ad94b158` by `git ls-remote`; a new clean isolated worktree was pinned to that exact commit. The primary checkout was not edited. Docker production build: `docker run --rm -u "$(id -u):$(id -g)" -v "$PWD:/src" -w /src deathleak-linux-build:bookworm make -C port -j4`. Run `sh port/tests/scheduled_multiscene_cave6_pixels.sh port/build/scheduled-multiscene-cave6` from the repository root. Recheck existing output with `DEATHLEAK_VERIFY_ONLY=1` and the same command.

## Result

Ordinary scripted controls `R:1100,Q:40,R:900`, starting from Forest, traversed authored triggers at frames 398 (waterfall top), 762 (waterfall bottom), 1378 (underwater cave), and 1780 (underwater cave 6), without warp, collision bypass, or injected state. The 1880-fixed-frame synthetic 60-Hz simulate-only and controlled-clock 144-Hz scheduled-production arms had identical fixed gameplay/combat/transition endpoints, except wall elapsed. End state: cave 6, health 25, 15 earned orbs, alive. Audio level dispatch appeared exactly once per visited scene.

The opt-in `--test-scheduled-refresh-slots` entered the actual `SealFreshBoundary`/`TryPresent` production branch. It reported 1880 scheduled fixed-boundary presents and 2655 due update-free presents, exactly accounting for 4535 service presents, 1463 fresh presents, and zero surface failures. The inherited expanded witness compared game/film/camera/shake/shader/event/combat/audio-route observables and collection/ownership sets across every update-free service call and scheduled boundary service call; 2655 update-free slots passed. It is a bounded observable witness, not exhaustive process-memory identity.

At cave-6 fixed frame 1800, due update-free slot 0 had fractional alpha 0.416667 and admitted 12 commands. Actual full 1280×720 32-bit post-HUD BMP equaled independently replayed sealed-packet BMP byte-for-byte, SHA-256 `a5a364c46918a912c0b3b02627f8547779662b45f02972aaa2b86557b1386685`; it differed from held-current at 567,596 pixels. Slot 1 had alpha zero, admitted zero, and correctly matched held-current, with its own full oracle match. The new script expressly asserts both outcomes and fails if no genuine fractional admitted slot is captured.

## Boundary

This proves controlled-clock offscreen llvmpipe functional behavior through four transitions on the scheduled branch. It does not establish physical selected >60-Hz, hardware input, NVE7 GPU cadence/FPS, sustained performance, or release acceptance. Connected LVDS is ~59.901 Hz and Xorg/device authority remains blocked. The production loop remains single-threaded, so fixed work can miss display deadlines. No atlas/bind work was done.
