# Death Leak Linux — timing and route acceptance pivot (2026-10-26)

Status: **in progress; release acceptance not met**. The software remains a partial port. This checkpoint supersedes atlas-coverage work as the next acceptance priority; it does not erase other work or claim a new FPS gain.

## Verified this cycle

- Built a detached, clean worktree at commit `a5e39ac` using `deathleak-linux-build:bookworm`, leaving the primary tree's pre-existing dirty files untouched.
- Added `port/tests/waterfall_cave_route.sh`. One uninterrupted Forest-1-spawn deterministic route, `R:1100,Q:40,R:900` at 1/60 s, entered Waterfall-top (frame 398), Waterfall-bottom (762), underwater cave (1378), and underwater cave 6 (1780). It ended alive with 25 health, 15 earned orbs, and balance 15. No warp, save load, injected money, or authored XML edit was used. The clean-build route gate passed.
- Continuation probes of ordinary right movement and held right-jump stop at cave-6 x≈1319.33. Authored invisible solid id 3 starts at x≈1383.81; subtracting player collision half-width ≈64.48 explains the stop. Its 1600-unit collision height makes a simple jump insufficient. This is a bounded route finding, not proof the original level is impossible or that collision should be bypassed.
- Concurrent remote commits through `c14a0fa` added read-only presentation-clock and fixed-update preparation seams for HUD, camera shake, shader clock, gameplay film, weather, loading overlay, and intro prompt. These are useful architectural progress, but the live loop still couples draw/present to updates; no >60-Hz emitted-frame or physical-display acceptance is established.

## Next acceptance gates

1. Find a source-backed cave-6 continuation around solid id 3 and extend the uninterrupted route through a later authored exit, with survival and earned-state checks. Do not remove or tunnel through the authored solid.
2. Split simulation from a side-effect-free presentation snapshot before claiming >60-Hz application output. Prove synchronized full post-HUD pixels, film clock, camera/shake behavior, and physical selected high-Hz mode if supported.
3. Run focused no-script real keyboard/pointer and representative combat/transition playtests on an unlocked, physical accelerated GPU display. The current sandbox cannot access the physical Xorg authority; offscreen llvmpipe is test-only and not performance/release acceptance.
4. Keep atlas micro-gates off the FPS-critical path unless a candidate demonstrates a plausible millisecond-scale win on the actual GPU without image mismatch.

Completion requires a real-display/GPU playtest and sustained performance evidence. Neither is claimed here.

## 2026-10-26 — update-free presentation service extraction (bounded slice)

- From clean isolated live remote `131fa3fa53c07f03c2948033f54529270c3e4ac8`, extracted the existing opt-in fullscreen refresh-only replay branch into `RefreshPresentationService`. It owns the display-slot scheduler and only consumes a sealed renderer command snapshot; its API has no LevelScene, input, audio, capture, film, shader or fixed-step clock dependency. Both surface-stability checks still fail closed. The ordinary fixed-60-Hz update/draw and capture order are unchanged. This is separation of the display-only consumer, **not** a full independent present loop or motion interpolation.
- Focused 100,000-repeat ordered-packet fixture passed, now including service no-snapshot, sealed replay/present, no extra capture/update, and positive next-update replacement. Refresh-slot scheduler fixture passed 60/75/120/144/240-Hz synthetic slots with 601 fixed updates. Established Docker production build passed. Uninterrupted Forest→Waterfall-top→bottom→underwater cave→cave 6 route passed with 15 earned orbs and 25 health. Offscreen llvmpipe route is functional only, not physical GPU acceptance.
- Next gate: move the ordinary presentation invocation behind the same independent service boundary, then prove synchronized full post-HUD Forest/Waterfall/combat/transition pixels and sustained selected >60-Hz mode on strict physical GPU. The existing `--refresh-slots` service remains opt-in, and no FPS, 8K, hardware-input or release claim follows from this refactor.
