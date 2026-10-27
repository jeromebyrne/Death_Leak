# Uninterrupted cave-6 admitted production extra-slot gate — 2026-10-26

## Scope and source-backed route

Clean detached worktree from freshly fetched live `f9b53ca`; primary dirty source checkout untouched. Docker production build of this scoped change: `bc257a01efa983979a2803a08d82a6fe5e970944f7038783e38bd53e0884c8b4` (SHA-256). Run `sh port/tests/uninterrupted_cave6_fresh_slot.sh` from repository root after `docker run --rm -u "$(id -u):$(id -g)" -v "$PWD:/src" -w /src deathleak-linux-build:bookworm make -C port -j4`.

Both arms start at authored Forest-1 and use the same ordinary-control `R:1100,Q:40,R:900` route for 1,880 fixed updates; there is no direct cave-6 load, save, warp, injected actor/state, XML modification, or collision bypass. Transition frames are Forest→Waterfall-top 398, Waterfall-top→bottom 762, bottom→underwater-cave 1378, and underwater-cave→cave 6 at 1780. Both arms end alive in cave 6 with 25 health, 15 earned orbs/balance, 0 hits/kills/shots, and identical gameplay/combat/transition log lines after excluding wall elapsed.

## Actual production slot and side-effect witness

The opt-in synthetic 144-Hz production `RefreshPresentationService::TryPresent` captured the **first admitted** extra slot after cave-6 frame 1800. At fixed frame 1800, slot 0, alpha 0.416667, 12 commands were admitted. The actual 1280×720 32-bit post-HUD backbuffer BMP was byte-identical to an independent explicit sealed-packet sampler/replay oracle (SHA-256 `b8edc7d85f1e23473035ba80d69384efd11e1cb9319b34e7ca8c471f32f55c7c`). It differed from the held-current backbuffer in 568,555 pixels. This is an admitted later-scene production slot reached by uninterrupted controls, not a direct scene-load or diagnostic-only sampler.

The opt-in `--verify-refresh-side-effects` invariant sampled game-owned fixed state before/after all **2,657 presented refresh-only slots**: scene epoch/level, player/motion/camera, prepared shake view/time, both HUD-film indices and last-switch clocks, menu film time, shader total/wobble clocks, preceding rendered delta, level/loading/final-scene event clocks, health/focus/hits/fired, and presentation scene generation. No slot changed this witness. Audio-level dispatch occurred exactly once per visited scene (five in route order); the extra-slot service has only sealed packet/renderer dependencies and no game/audio update call. Surface failures: 0. The 60-Hz simulate-only and 144-Hz rendered arms had identical fixed endpoints.

## Boundary

This is offscreen llvmpipe software rendering with synthetic refresh slots. `TEST_REFRESH_HZ=144` and present intervals are **not** selected physical display mode, actual GPU cadence, sustained FPS, hardware input/audio, or release acceptance. Connected LVDS is approximately 59.901 Hz and Xorg/device authority is unavailable. No atlas/bind optimization, admission-policy loosening, or frame-time gain is claimed. The fixed-state witness is bounded to the listed observables; it is not an exhaustive memory-state proof.
