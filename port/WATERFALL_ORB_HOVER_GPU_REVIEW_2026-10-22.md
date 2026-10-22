# Waterfall-top authored CurrencyOrb hover/tracking GPU review — 2026-10-22

## Result

Implemented a bounded waterfall-top *small authored* CurrencyOrb path. The 22 source XML orbs retain their art, alpha, dimensions, depth, animation and persistence keys. On simulation ticks in the source 4000×4000 camera update zone, idle Y uses its XML origin plus `y_offset * sin(phase)` with authored `amplitude` as phase speed. Phases are stable per normalized level path/XML ID for repeatable Linux captures, not a reconstruction of Windows `rand()`. The world-space XY drives both rendering and collection. Source-style strict `<130` acquisition is sticky; the trigger tick hovers, the first tracking tick takes the final base sine step, and steering then integrates prior acceleration before choosing a dominant-axis command. Tracking has source 2.5/4.8 acceleration, `>2s` multiplier, X cap 20, velocity rotation, and no origin snap. Collection is strict `<100` once, with existing saved-key suppression. The update runs before collection even in `--simulate-only`, independently of visual animation.

## Verification

- Docker production `make -j2` passed in `deathleak-linux-build:bookworm` (only existing TinyXML warnings). Docker focused `waterfall_orb_hover_tracking_test.cpp` and existing `orb_animation_zone_test.cpp` passed. The new CPU capture test covers repeatable phase, bounded Y motion, off-zone freeze, tracking handoff, motion, one-time collection at moved XY and saved-key hiding. `git diff --check` passed.
- Physical Xorg `:0`, nouveau/NVE7, strict SDL OpenGL accelerated (`accelerated=1 software=0`): fixed waterfall-top `--play`, `--script '.:120'`, 120 frames, `--fixed-dt 0.016666667`, dummy audio, produced full, hide-ID89, and only-ID89 BMPs/logs under ignored `port/build/waterfall-orb-hover-gpu-2026-10-22/`. Full vs hide-ID89 at the *same simulation state* differs in 1,025 RGB pixels, bbox x402–432/y371–414. All three logs end with player `(818.985,2933.64)`, one orb collected, health 25, balance 1. Full BMP SHA-256: `cd296aa194af4797d81920e8fe418c2eaad304e7482eed88d6962100f3a1d28a`.
- The previous pre-hover 120-frame full BMP differs in 14,592 pixels, bbox x70–1266/y90–493, but its balance is 0 (new movement leads to a pickup), so this cross-build comparison is **not** a matched gameplay-state parity measurement. The same-build hide-ID89 control is the valid fixed-state visual contribution check.

## Limits

The scene API exposes player position, not collision centre. Tracking aims at position and omits the Windows small-orb trail and SFX; exact Windows collision dispatch geometry, water effects, timing targetDelta and global RNG phase are not reproduced. The Linux A/B proves a rendered idle orb contribution and no GPU fallback, **not** matched Windows/Jerome visual acceptance. No matched Windows waterfall-top frame was available. Shared checkpoint/status files and unrelated reports were untouched.
