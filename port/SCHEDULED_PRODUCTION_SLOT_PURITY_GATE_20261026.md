# Actual scheduled production-present gate — 2026-10-26

## Provenance and reproducibility

New isolated detached worktree rebased onto the actual live GitHub branch head `16f8283e25594702f6e687a2b7df8d7f190d7a75` (the workspace's cached `origin/feature/linux-renderer-foundation` ref was stale at `ae3675b`; `git ls-remote`/`FETCH_HEAD` established live head). Primary dirty checkout untouched. Docker production build (`deathleak-linux-build:bookworm`) SHA-256 `3ef45c8aeb25b330f171d538c8943cd876dc02f1d0ffd2170c12f86dda69a04e`. Run `sh port/tests/scheduled_production_slot_pixels.sh port/build/scheduled-production-slot` after building. Focused presentation-command-packet, refresh-slot-scheduler and fresh-presentation-packet fixtures passed.

## Bounded implementation

`--test-scheduled-refresh-slots` is opt-in and valid only with the existing software controlled-clock refresh harness. It enters the **same production branch** added at `16f8283`: fixed updates call `SealFreshBoundary` and `TryPresent`; update-free iterations call `TryPresent` on due display slots. The only clock substitution is the deterministic synthetic tick at the fixed-boundary call. Historical synthetic tests keep their earlier mandatory-boundary branch. `--verify-refresh-side-effects` now compares 44 numeric game/film/camera/shake/shader/combat/event observables plus exact current/audio level, six collection/ownership sets and combat trigger-history around both scheduled fixed-boundary and update-free service calls. The first-admitted capture selector now requires a genuinely fractional alpha; it cannot silently select an alpha-zero sample. No production admission, atlas or bind policy changed.

## Observed actual scheduled path

At 144-Hz controlled slots over 80 fixed Forest updates, the production scheduling branch reported 80 fixed-boundary presents and 141 update-free presents, 21 fresh presents, zero surface failures. The first selected **due update-free** present at fixed frame 58, slot 0, alpha 0.416667 admitted 57 commands. Actual 1280×720 32-bit post-HUD backbuffer matched independent sealed-packet replay byte-for-byte (SHA-256 `334c3c0f0d2d8755e4580ada68bfe1924991754d0a3e86e4aa6bdd24194e68b6`) and differed from held-current at 753,890 pixels. A second fractional slot (alpha .833333, 57 commands) also matched the oracle and differed from held at 700,077 pixels; a later alpha-zero slot conservatively held exactly. All 141 update-free slots and all scheduled fixed-boundary service calls passed the expanded state witness. The service's 221 total presents equal 80 boundary plus 141 update-free calls. The synthetic 60-Hz simulate-only arm had the exact same fixed gameplay/combat/transition endpoint as the scheduled 144-Hz arm (wall elapsed excluded).

A separate controlled 30-Hz branch sanity run reported 12 fixed-boundary and 5 update-free presents, with state purity and zero surface failures. Its event count is only a branch sanity check, not a guarantee that a fixed boundary was withheld on this particular short route; the independent scheduler/packet spy fixture covers non-due behavior.

Earlier uninterrupted Forest→cave-6 gate on the pre-`16f8283` synthetic boundary path passed with the expanded witness over 2,659 update-free slots and exact cave-6 post-HUD pixels, but **does not** prove the new scheduled branch. Its result is kept separate from this actual scheduled-path proof.

## Boundary

This is offscreen llvmpipe controlled-clock functional correctness, not physical selected >60-Hz, NVE7 GPU cadence, sustained FPS, hardware input/audio or release acceptance. The production path remains single-threaded; fixed work may miss display deadlines. Connected LVDS is ~59.901 Hz and Xorg/device authority is unavailable. The witness is bounded to listed observables; it is not exhaustive process-memory identity. No performance gain claimed.
