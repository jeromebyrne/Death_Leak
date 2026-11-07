# Death Leak Linux performance goal

Owner: OpenClaw performance workstream. Started 2026-10-25. Status: active, not accepted.

## Outcome

Maximize sustained active-gameplay frame rate on the real NVE7 display at 1280×720 with hardware-accelerated rendering, without reducing visual fidelity, game logic, resolution, or required effects. First gate: <=16.667 ms median (>=60 FPS) and controlled tail latency across representative Forest, Waterfall, combat, and transition routes. Continue optimizing beyond 60 FPS where repeatable gains preserve parity. No software-rendering or headless-only acceptance.

## Current evidence

- Forest active route: 20.1275 ms median, about 50 FPS, on physical NVE7; the full-scene GPU path dominates CPU update.
- Far-background ID 1 is the largest individually removable scene layer (~1.5–1.6 ms), but removal changes authored imagery. Its texture has full nonzero alpha, so transparent-border culling is not useful.
- Previously shipped render-target and text-batching changes yielded measured gains. Backplane-copy cache was slower and changed pixels; rejected.
- The long scripted route stalls in Waterfall-bottom; its performance numbers are not full-campaign acceptance.

## Work program

1. Preserve and reconcile the dirty source tree. Use isolated diagnostic sources/builds; never overwrite unrelated changes or race another GPU owner.
2. Repeat an uncontended physical-GPU baseline with exact binary/route/settings, p50/p95/p99, slow-frame and pass timings; keep control runs interleaved with candidates.
3. Completed ID-1 scissor/visible-region diagnostic on physical NVE7: full/half/zero coverage frame medians 20.16/19.49/18.69 ms. Fragment/coverage cost is material; the scissor itself changes pixels and is not a candidate fix. Derive conservative opaque-coverage masks before attempting any copy-free occlusion treatment.
4. Exhaust measured, visually lossless avenues in priority order: occlusion-aware scene painter/geometry; batching and state-change reduction; texture sampling and formats; shader/pass fusion; framebuffer/target traffic; upload/readback avoidance; SDL/OpenGL presentation/compositor path; driver and GPU power-state controls. For each, record predicted mechanism, GPU A/B, exact-frame pixel checks, and keep/reject result. Do not optimize based on CPU wall-time phase labels alone.
5. Repair the Waterfall-bottom scripted traversal gate separately, then run sustained physical-display routes through all available scene types and transitions. Track median and tail latency, frame pacing, memory/VRAM, and thermal behavior.
6. Commit and push only verified, scoped source changes; leave failed prototypes as documented experiments. Update the checkpoint after each bounded step and proactively report meaningful gains or genuine blockers in the originating DM.

## Acceptance / stop condition

Performance work is complete only after representative sustained physical-GPU play is measured, visuals and gameplay remain acceptable, and the highest practicable stable frame rate is demonstrated with evidence. A 60-FPS median alone does not establish release readiness if p95/p99 or transition stalls remain bad. If a hardware/driver ceiling prevents the target, document measured alternatives and tradeoffs rather than claiming success.

## Next action

Commit `f888a96` replaced Waterfall-top ID58's viewport-covering 2×2 white repeat-X texture with an exact-pixel solid blend, yielding repeated physical-NVE7 mixed-route p50 29.655/28.913 ms controls versus 21.090/21.045 ms candidates; p95 31.524/31.732 versus 22.894/22.864 ms. Top ordinal 700 lossless RGB matched serial control and production capture exactly, but same-binary visual-clock variance exists in other runs. Transition max remains ~99 ms, and neither 60 FPS nor sustained release acceptance is evidenced. Next: remeasure current-tree scene-tagged GPU bands after ID58; split the remaining 3000–1001 painter interval by ordered draw/SDL-private spans, test only pixel-preserving candidates, and independently run the repaired 1,500+ frame bottom→cave route with scene/tail statistics. Preserve unrelated dirty work and one physical GPU owner.

## Scope expansion — 2026-10-25
Jerome authorized 4K atlas coverage, maximum safe draw batching, shader optimization, up-to-8K display resolution and device-supported refresh rates with game-logic timing rework. The implementation contract, starting inventory and first vertical slices are in PORT_ATLAS_SCALING_GOAL.md. This new scope supersedes the earlier 1280×720/60-Hz-only outcome; do not claim completion from old benchmarks.
