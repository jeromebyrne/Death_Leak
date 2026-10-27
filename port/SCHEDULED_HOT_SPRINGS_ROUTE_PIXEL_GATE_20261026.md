# Scheduled production Hot Springs route pixel gate — 2026-10-26

## Scope

New isolated worktree pinned to live GitHub `feature/linux-renderer-foundation` commit `c9fac842cda231bf0e8330d67f891931f383a1b8` as checked by `git ls-remote`; primary dirty tree untouched. `port/tests/scheduled_hot_springs_pixels.sh` extends the scheduled cave-6 production gate using uninterrupted ordinary controls `R:1100,Q:40,R:640,E:1500,R:1300` from Forest to authored Hot Springs. It asserts the five source-backed transition frames (398, 762, 1378, 1780, 3037), a live fixed-60-Hz endpoint, exact synthetic 60/144-Hz gameplay/combat/transition parity, due update-free production `SealFreshBoundary`/`TryPresent` calls, full 1280×720 post-HUD actual-vs-independent sealed-oracle bytes, non-held admitted pixels, audio dispatch once per scene, and bounded update-free game/film/camera/shake/shader/event/state purity. No warp, save, injection, XML edit or collision bypass.

## Result

Docker production build passed, SHA-256 `3ef45c8aeb25b330f171d538c8943cd876dc02f1d0ffd2170c12f86dda69a04e`. Focused presentation-command-packet, refresh-slot-scheduler and fresh-presentation-packet fixtures passed. Run `sh port/tests/scheduled_hot_springs_pixels.sh port/build/scheduled-hot-springs` from this repository; `DEATHLEAK_VERIFY_ONLY=1` rechecks captured output.

The 3,090-fixed-frame route passed five authored transitions, ending alive in Hot Springs with health 25, 16 earned orbs/balance, no hits/kills/shots. Synthetic 60-Hz simulate-only and 144-Hz scheduled production-render arms had identical fixed gameplay/combat/transition endpoint lines after excluding wall elapsed. Exactly one audio level dispatch appeared per visited scene. The actual scheduled branch reported 3,090 fixed-boundary presents plus 4,399 due update-free presents = 7,489 service presents; bounded side-effect witness passed all 4,399 update-free slots, with zero surface failures.

At Hot Springs fixed frame 3050, due update-free production slot 0 at alpha .416667 admitted 38 commands. Its actual complete 1280×720 32-bit post-HUD BMP was byte-identical to independently sampled sealed-packet oracle, SHA-256 `804036ce296fde2f04ecedf30cacc92da47b942d13c16eaafe741cb3a8b0b72d`, and differed from held-current at 656,425 pixels. Slot 1 at alpha zero also admitted 38 commands; actual/oracle BMPs were byte-identical (SHA-256 `d31746f65fd0e7caf3559257680e3245c2d1a7e6c114d8c14cb9f6e09e684273`) and differed from held-current at 704,523 pixels. The test does not equate alpha zero with a held packet: admission and oracle pixels, not alpha alone, determine the expected result. The fractional slot alone satisfies the positive fresh-motion gate.

This extends the prior scheduled cave-6 pixel gate to a later authored scene on the same uninterrupted route. It does not prove all Hot Springs frames, player-to-enemy combat here, or full-memory purity.

## Boundary

Offscreen llvmpipe and controlled-clock synthetic slots are correctness-only evidence, not physical selected >60-Hz, NVE7 GPU cadence/FPS, hardware input, or release acceptance. Connected LVDS is ~59.901 Hz and Xorg/device authority remains blocked. The state witness is bounded, not exhaustive memory identity. Atlas/bind microgates remain paused.
