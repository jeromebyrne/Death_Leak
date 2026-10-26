# Death Leak Linux — timing and route acceptance pivot (2026-10-26)

Status: **in progress; release acceptance not met**. The software remains a partial port. This checkpoint supersedes atlas-coverage work as the next acceptance priority; it does not erase other work or claim a new FPS gain.

## Verified this cycle

- Built a detached, clean worktree at commit `a5e39ac` using `deathleak-linux-build:bookworm`, leaving the primary tree's pre-existing dirty files untouched.
- Added `port/tests/waterfall_cave_route.sh`. One uninterrupted Forest-1-spawn deterministic route, `R:1100,Q:40,R:900` at 1/60 s, entered Waterfall-top (frame 398), Waterfall-bottom (762), underwater cave (1378), and underwater cave 6 (1780). It ended alive with 25 health, 15 earned orbs, and balance 15. No warp, save load, injected money, or authored XML edit was used. The clean-build route gate passed.
- Continuation probes of ordinary right movement and held right-jump stop at cave-6 x≈1319.33. Authored invisible solid id 3 starts at x≈1383.81; subtracting player collision half-width ≈64.48 explains the stop. Its 1600-unit collision height makes a simple jump insufficient. This is a bounded route finding, not proof the original level is impossible or that collision should be bypassed.
- The existing fixed-60-Hz gameplay slice has synthetic-rate CPU evidence, but the separate >60-Hz presentation prototype was rejected. The render loop still couples draw/present to updates; high-refresh acceptance has not advanced in this cycle.

## Next acceptance gates

1. Find a source-backed cave-6 continuation around solid id 3 and extend the uninterrupted route through a later authored exit, with survival and earned-state checks. Do not remove or tunnel through the authored solid.
2. Split simulation from a side-effect-free presentation snapshot before claiming >60-Hz application output. Prove synchronized full post-HUD pixels, film clock, camera/shake behavior, and physical selected high-Hz mode if supported.
3. Run focused no-script real keyboard/pointer and representative combat/transition playtests on an unlocked, physical accelerated GPU display. The current sandbox cannot access the physical Xorg authority; offscreen llvmpipe is test-only and not performance/release acceptance.
4. Keep atlas micro-gates off the FPS-critical path unless a candidate demonstrates a plausible millisecond-scale win on the actual GPU without image mismatch.

Completion requires a real-display/GPU playtest and sustained performance evidence. Neither is claimed here.
